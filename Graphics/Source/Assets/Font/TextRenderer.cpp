/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"

#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/GlyphBuffers.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/EmbeddedShaders.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#define TEXT_RENDERER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::TextRenderer, handle )

using namespace DenOfIz;
using namespace DirectX;

namespace
{
    constexpr float    DefaultFontSize  = 16.0f;
    constexpr uint32_t DefaultNumFrames = 3;
    constexpr uint32_t MinQuadCapacity  = 1024;
    constexpr uint64_t ShapeCacheAge    = 240;
    // Set on a quad's glyph when it samples a colour glyph bitmap, matches the text shaders.
    constexpr uint32_t ColourGlyphBit = 0x80000000u;
    // t4, after the four glyph buffers.
    constexpr uint32_t QuadBinding = 4;

    uint32_t GrowCapacity( const uint32_t current, const uint32_t required, const uint32_t minimum )
    {
        uint32_t capacity = std::max( current, minimum );
        while ( capacity < required )
        {
            capacity *= 2;
        }
        return capacity;
    }

    uint8_t BaseLevel( const DenOfIz_TextDirection direction )
    {
        switch ( direction )
        {
        case DENOFIZ_TEXT_DIRECTION_LEFT_TO_RIGHT:
            return 0;
        case DENOFIZ_TEXT_DIRECTION_RIGHT_TO_LEFT:
            return 1;
        default:
            return TextAutoLevel;
        }
    }
} // namespace

namespace DenOfIz
{
    class TextRenderer
    {
        // One per glyph, the vertex shader expands it into the four corners of its quad. Bounds = (left, top, right, bottom) in em space, or in texels
        // for a colour glyph bitmap. EmsPerUnit is used by SlugDilate.
        struct Quad
        {
            DenOfIz_Float4 Rect;
            DenOfIz_Float4 Bounds;
            DenOfIz_Float4 Color;
            DenOfIz_Float2 EmsPerUnit;
            uint32_t       Glyph;
            uint32_t       Padding;
        };
        static_assert( sizeof( Quad ) == 64 && offsetof( Quad, Bounds ) == 16 && offsetof( Quad, Color ) == 32 && offsetof( Quad, EmsPerUnit ) == 48 &&
                       offsetof( Quad, Glyph ) == 56 );

        struct Uniforms
        {
            DenOfIz_Float4x4 Projection;
            DenOfIz_Float4   Viewport;
        };
        static_assert( sizeof( Uniforms ) == 80 && offsetof( Uniforms, Viewport ) == 64 );

        // Em units, each line in visual order. Record is the ColourGlyphStore source of a colour glyph, the SlugGlyphStore record otherwise.
        struct ShapedGlyph
        {
            const Font *FontPtr;
            uint32_t    GlyphId;
            float       Advance;
            float       OffsetX;
            float       OffsetY;
            uint32_t    Record;
            bool        Colour;
        };

        // Above and Below the baseline in em, the tallest of the line's fonts for line-height normal.
        struct ShapedLine
        {
            uint32_t GlyphStart;
            uint32_t GlyphEnd;
            float    Advance;
            uint32_t Spaced;
            float    Above;
            float    Below;
        };

        struct ShapedBlock
        {
            std::vector<ShapedGlyph> Glyphs;
            std::vector<ShapedLine>  Lines;
            uint64_t                 LastUsed = 0;
        };

        struct ShapeKey
        {
            std::string           Text;
            uint16_t              FontId;
            DenOfIz_TextDirection Direction;

            bool operator==( const ShapeKey &other ) const = default;
        };

        struct ShapeKeyHash
        {
            size_t operator( )( const ShapeKey &key ) const
            {
                return std::hash<std::string>{ }( key.Text ) ^ static_cast<size_t>( key.FontId ) << 1 ^ static_cast<size_t>( key.Direction ) << 17;
            }
        };

        // LineHeight and Baseline are zero for line-height normal, each line then has its own.
        struct LineMetrics
        {
            Font *FontPtr;
            float Size;
            float Scale;
            float LineHeight;
            float Baseline;
        };

        struct Frame
        {
            DenOfIz_Buffer    QuadBuffer      = DENOFIZ_NULL_HANDLE;
            Quad             *QuadData        = nullptr;
            uint32_t          QuadCapacity    = 0;
            DenOfIz_BindGroup BindGroup       = DENOFIZ_NULL_HANDLE;
            uint32_t          GlyphGeneration = 0;
        };

        DenOfIz_LogicalDevice                                   m_device;
        SlugGlyphStore                                          m_glyphs;
        ColourGlyphStore                                        m_colourGlyphs;
        GlyphBuffers                                            m_glyphBuffers;
        TextShaper                                              m_shaper;
        ShapedText                                              m_shaped;
        std::vector<uint8_t>                                    m_levels;
        std::vector<uint32_t>                                   m_order;
        DenOfIz_ShaderProgram                                   m_shaderProgram = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout>                    m_bindGroupLayouts;
        DenOfIz_RootSignature                                   m_rootSignature   = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                                     m_inputLayout     = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                                        m_pipeline        = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                                          m_uniformBuffer   = DENOFIZ_NULL_HANDLE;
        uint8_t                                                *m_uniformData     = nullptr;
        uint32_t                                                m_alignedUniforms = 0;
        DenOfIz_Buffer                                          m_indexBuffer     = DENOFIZ_NULL_HANDLE;
        uint32_t                                                m_quadCapacity    = 0;
        std::vector<Frame>                                      m_frames;
        uint32_t                                                m_frameIndex = 0;
        uint64_t                                                m_batch      = 0;
        std::map<uint16_t, Font *>                              m_fonts;
        std::map<uint16_t, std::vector<uint16_t>>               m_fallbacks;
        std::vector<Font *>                                     m_chain;
        std::unordered_map<ShapeKey, ShapedBlock, ShapeKeyHash> m_shapes;
        ShapeKey                                                m_shapeLookup{ }; // Reused so looking text up does not allocate.
        std::vector<Quad>                                       m_quads;
        DenOfIz_Float4x4                                        m_projection{ };
        DenOfIz_Viewport                                        m_viewport{ };

    public:
        explicit TextRenderer( const DenOfIz_TextRendererDesc &desc );
        ~TextRenderer( );

        void SetProjectionMatrix( const DenOfIz_Float4x4 &projection );
        void SetViewport( const DenOfIz_Viewport &viewport );

        uint16_t AddFont( Font *font, uint16_t fontId );
        Font    *GetFont( uint16_t fontId ) const;
        void     RemoveFont( uint16_t fontId );
        void     SetFallbacks( uint16_t fontId, std::vector<uint16_t> fallbacks );

        void           BeginBatch( );
        void           AddText( const DenOfIz_TextRenderDesc &desc );
        void           EndBatch( DenOfIz_CommandList commandList );
        DenOfIz_Float2 MeasureText( DenOfIz_StringView text, const DenOfIz_TextRenderDesc &desc );

    private:
        void               Invalidate( uint16_t fontId );
        static float       LineWidth( const ShapedLine &line, float scale, float spacing );
        static float       LineHeight( const LineMetrics &metrics, const ShapedLine &line );
        static float       TextHeight( const LineMetrics &metrics, const ShapedBlock &block );
        bool               Metrics( const DenOfIz_TextRenderDesc &desc, LineMetrics &metrics ) const;
        const ShapedBlock &Shape( DenOfIz_StringView text, uint16_t fontId, Font *font, DenOfIz_TextDirection direction );
        void               EmitGlyph( const LineMetrics &metrics, const ShapedGlyph &glyph, float x, float baseline, const DenOfIz_Float4 &color );
        void               EmitColourGlyph( uint32_t source, float size, float x, float y, const DenOfIz_Float4 &color );
        float              PixelsPerEm( float x, float y, float size ) const;
        void               EnsureQuadCapacity( uint32_t quads );
        void               EnsureFrameCapacity( Frame &frame, uint32_t index, uint32_t quads ) const;
        void               BindFrame( Frame &frame, uint32_t index ) const;
    };
} // namespace DenOfIz

TextRenderer::TextRenderer( const DenOfIz_TextRendererDesc &desc ) : m_device( desc.LogicalDevice ), m_glyphBuffers( desc.LogicalDevice )
{
    m_shaderProgram = EmbeddedShaders::GetTextRendererShaderProgram( );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_shaderProgram, &reflectDesc );
    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_device, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }
    DenOfIz_RootSignatureDesc rootSignatureDesc{ };
    rootSignatureDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSignatureDesc.BindGroupLayouts.NumElements = static_cast<uint32_t>( m_bindGroupLayouts.size( ) );
    rootSignatureDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_device, &rootSignatureDesc, &m_rootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( m_device, &reflectDesc.InputLayout, &m_inputLayout );

    DenOfIz_RenderTargetDesc renderTarget{ };
    renderTarget.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    renderTarget.Blend.Enable                = true;
    renderTarget.Blend.SrcBlend              = DENOFIZ_BLEND_SRC_ALPHA;
    renderTarget.Blend.DstBlend              = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOp               = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.SrcBlendAlpha         = DENOFIZ_BLEND_ONE;
    renderTarget.Blend.DstBlendAlpha         = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOpAlpha          = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.RenderTargetWriteMask = 0x0F;

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.RootSignature                      = m_rootSignature;
    pipelineDesc.InputLayout                        = m_inputLayout;
    pipelineDesc.ShaderProgram                      = m_shaderProgram;
    pipelineDesc.BindPoint                          = DENOFIZ_BIND_POINT_GRAPHICS;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.CullMode                  = DENOFIZ_CULL_MODE_NONE;
    pipelineDesc.Graphics.DepthTest.Enable          = false;
    pipelineDesc.Graphics.DepthTest.Write           = false;
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;
    DenOfIz_LogicalDevice_CreatePipeline( m_device, &pipelineDesc, &m_pipeline );

    m_frames.resize( desc.NumFrames > 0 ? desc.NumFrames : DefaultNumFrames );
    m_alignedUniforms = Utilities::Align( sizeof( Uniforms ), 256 );
    DenOfIz_BufferDesc uniformDesc{ };
    uniformDesc.NumBytes  = static_cast<size_t>( m_alignedUniforms ) * m_frames.size( );
    uniformDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    uniformDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    uniformDesc.DebugName = DENOFIZ_STRING( "Text Uniforms" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &uniformDesc, &m_uniformBuffer );
    DenOfIz_Buffer_MapMemory( m_uniformBuffer, reinterpret_cast<void **>( &m_uniformData ) );

    for ( uint32_t i = 0; i < m_frames.size( ); ++i )
    {
        DenOfIz_BindGroupDesc bindGroupDesc{ };
        bindGroupDesc.Layout = m_bindGroupLayouts[ 0 ];
        DenOfIz_LogicalDevice_CreateBindGroup( m_device, &bindGroupDesc, &m_frames[ i ].BindGroup );
        EnsureFrameCapacity( m_frames[ i ], i, MinQuadCapacity );
    }

    m_fonts[ 0 ] = DENOFIZ_HANDLE_IS_VALID( desc.Font ) ? FONT_IMPL( desc.Font ) : Font::Default( );
    SetViewport( DenOfIz_Viewport{ 0.0f, 0.0f, static_cast<float>( desc.Width ), static_cast<float>( desc.Height ) } );
}

TextRenderer::~TextRenderer( )
{
    DenOfIz_LogicalDevice_WaitIdle( m_device );
    for ( const Frame &frame : m_frames )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( frame.QuadBuffer ) )
        {
            DenOfIz_Buffer_UnmapMemory( frame.QuadBuffer );
            DenOfIz_Buffer_Destroy( frame.QuadBuffer );
        }
        DenOfIz_BindGroup_Destroy( frame.BindGroup );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
    DenOfIz_Buffer_UnmapMemory( m_uniformBuffer );
    DenOfIz_Buffer_Destroy( m_uniformBuffer );
    DenOfIz_Pipeline_Destroy( m_pipeline );
    DenOfIz_RootSignature_Destroy( m_rootSignature );
    for ( const DenOfIz_BindGroupLayout layout : m_bindGroupLayouts )
    {
        DenOfIz_BindGroupLayout_Destroy( layout );
    }
    DenOfIz_InputLayout_Destroy( m_inputLayout );
}

void TextRenderer::SetProjectionMatrix( const DenOfIz_Float4x4 &projection )
{
    m_projection = projection;
}

void TextRenderer::SetViewport( const DenOfIz_Viewport &viewport )
{
    m_viewport = viewport;
    XMFLOAT4X4 ortho{ };
    XMStoreFloat4x4( &ortho, XMMatrixOrthographicOffCenterLH( 0.0f, viewport.Width, viewport.Height, 0.0f, 0.0f, 1.0f ) );
    std::memcpy( &m_projection, &ortho, sizeof( DenOfIz_Float4x4 ) );
}

uint16_t TextRenderer::AddFont( Font *font, const uint16_t fontId )
{
    Invalidate( fontId );
    m_fonts[ fontId ] = font;
    return fontId;
}

Font *TextRenderer::GetFont( const uint16_t fontId ) const
{
    const auto it = m_fonts.find( fontId );
    return it != m_fonts.end( ) ? it->second : nullptr;
}

void TextRenderer::RemoveFont( const uint16_t fontId )
{
    m_fonts.erase( fontId );
    Invalidate( fontId );
}

void TextRenderer::SetFallbacks( const uint16_t fontId, std::vector<uint16_t> fallbacks )
{
    m_fallbacks[ fontId ] = std::move( fallbacks );
    Invalidate( fontId );
}

// Drops the shaped text drawn with the font, as primary or fallback.
void TextRenderer::Invalidate( const uint16_t fontId )
{
    auto uses = [ this, fontId ]( const uint16_t id )
    {
        const auto it = m_fallbacks.find( id );
        return id == fontId || ( it != m_fallbacks.end( ) && std::ranges::find( it->second, fontId ) != it->second.end( ) );
    };
    std::erase_if( m_shapes, [ &uses ]( const auto &entry ) { return uses( entry.first.FontId ); } );
}

void TextRenderer::BeginBatch( )
{
    m_quads.clear( );
    m_colourGlyphs.BeginFrame( );
    m_frameIndex = ( m_frameIndex + 1 ) % static_cast<uint32_t>( m_frames.size( ) );
    if ( ++m_batch % ShapeCacheAge == 0 )
    {
        std::erase_if( m_shapes, [ this ]( const auto &entry ) { return entry.second.LastUsed + ShapeCacheAge < m_batch; } );
    }
}

void TextRenderer::AddText( const DenOfIz_TextRenderDesc &desc )
{
    LineMetrics metrics{ };
    if ( !Metrics( desc, metrics ) )
    {
        return;
    }
    const ShapedBlock &block = Shape( desc.Text, desc.FontId, metrics.FontPtr, desc.Direction );

    float y = desc.Y;
    if ( desc.VerticalCenter )
    {
        y -= TextHeight( metrics, block ) * 0.5f;
    }
    for ( const ShapedLine &line : block.Lines )
    {
        float x = desc.X;
        if ( desc.HorizontalCenter )
        {
            x -= LineWidth( line, metrics.Scale, desc.LetterSpacing ) * 0.5f;
        }
        const float baseline = y + ( metrics.LineHeight > 0.0f ? metrics.Baseline : line.Above * metrics.Size );
        for ( uint32_t i = line.GlyphStart; i < line.GlyphEnd; ++i )
        {
            const ShapedGlyph &glyph = block.Glyphs[ i ];
            EmitGlyph( metrics, glyph, x, baseline, desc.Color );
            x += glyph.Advance * metrics.Scale + ( glyph.Advance != 0.0f ? desc.LetterSpacing : 0.0f );
        }
        y += LineHeight( metrics, line );
    }
}

void TextRenderer::EndBatch( const DenOfIz_CommandList commandList )
{
    if ( m_quads.empty( ) )
    {
        return;
    }
    Frame &frame = m_frames[ m_frameIndex ];
    m_glyphBuffers.Write( m_glyphs, m_colourGlyphs );
    const auto quadCount = static_cast<uint32_t>( m_quads.size( ) );
    EnsureQuadCapacity( quadCount );
    EnsureFrameCapacity( frame, m_frameIndex, quadCount );
    std::memcpy( frame.QuadData, m_quads.data( ), m_quads.size( ) * sizeof( Quad ) );

    const Uniforms uniforms{ m_projection, { m_viewport.Width, m_viewport.Height, 0.0f, 0.0f } };
    std::memcpy( m_uniformData + static_cast<size_t>( m_frameIndex ) * m_alignedUniforms, &uniforms, sizeof( Uniforms ) );
    if ( frame.GlyphGeneration != m_glyphBuffers.Generation( ) )
    {
        BindFrame( frame, m_frameIndex );
    }

    DenOfIz_CommandList_BindPipeline( commandList, m_pipeline );
    DenOfIz_CommandList_BindViewport( commandList, m_viewport.X, m_viewport.Y, m_viewport.Width, m_viewport.Height );
    DenOfIz_CommandList_BindGroup( commandList, frame.BindGroup );
    DenOfIz_CommandList_BindIndexBuffer( commandList, m_indexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );
    DenOfIz_CommandList_DrawIndexed( commandList, quadCount * 6, 1, 0, 0, 0 );
}

DenOfIz_Float2 TextRenderer::MeasureText( const DenOfIz_StringView text, const DenOfIz_TextRenderDesc &desc )
{
    LineMetrics metrics{ };
    if ( !Metrics( desc, metrics ) )
    {
        return DenOfIz_Float2{ 0.0f, 0.0f };
    }
    const ShapedBlock &block = Shape( text, desc.FontId, metrics.FontPtr, desc.Direction );
    float              width = 0.0f;
    for ( const ShapedLine &line : block.Lines )
    {
        width = std::max( width, LineWidth( line, metrics.Scale, desc.LetterSpacing ) );
    }
    return DenOfIz_Float2{ width, TextHeight( metrics, block ) };
}

float TextRenderer::LineWidth( const ShapedLine &line, const float scale, const float spacing )
{
    return line.Advance * scale + spacing * static_cast<float>( std::max( line.Spaced, 1u ) - 1 );
}

float TextRenderer::LineHeight( const LineMetrics &metrics, const ShapedLine &line )
{
    return metrics.LineHeight > 0.0f ? metrics.LineHeight : ( line.Above + line.Below ) * metrics.Size;
}

float TextRenderer::TextHeight( const LineMetrics &metrics, const ShapedBlock &block )
{
    float height = 0.0f;
    for ( const ShapedLine &line : block.Lines )
    {
        height += LineHeight( metrics, line );
    }
    return height;
}

bool TextRenderer::Metrics( const DenOfIz_TextRenderDesc &desc, LineMetrics &metrics ) const
{
    const auto it = m_fonts.find( desc.FontId );
    if ( it == m_fonts.end( ) )
    {
        spdlog::error( "TextRenderer: font id {} is not registered", desc.FontId );
        return false;
    }
    const DenOfIz_FontMetrics &font = it->second->Metrics( );
    metrics.FontPtr                 = it->second;
    metrics.Size                    = desc.FontSize > 0.0f ? desc.FontSize : DefaultFontSize;
    metrics.Scale                   = metrics.Size;
    metrics.LineHeight              = desc.LineHeight > 0.0f ? desc.LineHeight : 0.0f;
    metrics.Baseline                = desc.LineHeight > 0.0f ? ( desc.LineHeight - ( font.Ascent + font.Descent ) * metrics.Size ) * 0.5f + font.Ascent * metrics.Size : 0.0f;
    return true;
}

// Shaping happens in em, so one result serves every font size. Every paragraph is one line, laid out in visual order.
const TextRenderer::ShapedBlock &TextRenderer::Shape( const DenOfIz_StringView text, const uint16_t fontId, Font *font, const DenOfIz_TextDirection direction )
{
    const std::string_view chars = text.Chars != nullptr ? std::string_view( text.Chars, text.NumChars ) : std::string_view( );
    m_shapeLookup.Text.assign( chars );
    m_shapeLookup.FontId    = fontId;
    m_shapeLookup.Direction = direction;
    if ( const auto found = m_shapes.find( m_shapeLookup ); found != m_shapes.end( ) )
    {
        found->second.LastUsed = m_batch;
        return found->second;
    }
    ShapedBlock &block = m_shapes[ m_shapeLookup ];
    block.LastUsed     = m_batch;

    m_chain.assign( 1, font );
    if ( const auto fallbacks = m_fallbacks.find( fontId ); fallbacks != m_fallbacks.end( ) )
    {
        for ( const uint16_t fallback : fallbacks->second )
        {
            Font *chained = GetFont( fallback );
            if ( chained != nullptr && std::ranges::find( m_chain, chained ) == m_chain.end( ) )
            {
                m_chain.push_back( chained );
            }
        }
    }
    m_shaper.Shape( chars.data( ), static_cast<uint32_t>( chars.size( ) ), m_chain, BaseLevel( direction ), m_shaped );
    const std::vector<TextGlyph> &glyphs = m_shaped.Glyphs;
    block.Glyphs.reserve( glyphs.size( ) );
    for ( const TextParagraph &paragraph : m_shaped.Paragraphs )
    {
        const uint32_t count = paragraph.GlyphEnd - paragraph.GlyphStart;
        m_levels.resize( count );
        m_order.resize( count );
        TextVisualOrder( glyphs.data( ) + paragraph.GlyphStart, count, paragraph.Level, m_levels.data( ), m_order.data( ) );
        ShapedLine line{ static_cast<uint32_t>( block.Glyphs.size( ) ), 0, 0.0f, 0, 0.0f, 0.0f };
        auto       include = [ &line ]( const Font *used )
        {
            const DenOfIz_FontMetrics &metrics = used->Metrics( );
            line.Above                         = std::max( line.Above, metrics.Ascent + metrics.LineGap * 0.5f );
            line.Below                         = std::max( line.Below, metrics.Descent + metrics.LineGap * 0.5f );
        };
        include( font );
        for ( const uint32_t i : m_order )
        {
            const TextGlyph &glyph = glyphs[ paragraph.GlyphStart + i ];
            const Font      *used   = m_shaped.Fonts[ glyph.Font ];
            uint32_t         record = 0;
            bool             colour = false;
            if ( glyph.GlyphId != TextHiddenGlyph )
            {
                record = m_colourGlyphs.Find( used, glyph.GlyphId );
                colour = record != ColourGlyphStore::None;
                record = colour ? record : m_glyphs.Get( used->Id( ), used->HbFont( ), used->Upem( ), glyph.GlyphId );
            }
            block.Glyphs.push_back( ShapedGlyph{ used, glyph.GlyphId, glyph.Advance, glyph.OffsetX, glyph.OffsetY, record, colour } );
            line.Advance += glyph.Advance;
            line.Spaced += glyph.Advance != 0.0f ? 1 : 0;
            if ( glyph.GlyphId != TextHiddenGlyph && glyph.Font != 0 )
            {
                include( m_shaped.Fonts[ glyph.Font ] );
            }
        }
        line.GlyphEnd = static_cast<uint32_t>( block.Glyphs.size( ) );
        block.Lines.push_back( line );
    }
    return block;
}

void TextRenderer::EmitGlyph( const LineMetrics &metrics, const ShapedGlyph &glyph, const float x, const float baseline, const DenOfIz_Float4 &color )
{
    if ( glyph.GlyphId == TextHiddenGlyph )
    {
        return;
    }
    // Object space is y down, em space is y up.
    const float size = metrics.Size;
    const float ox   = x + glyph.OffsetX * metrics.Scale;
    const float oy   = baseline - glyph.OffsetY * metrics.Scale;
    if ( glyph.Colour )
    {
        EmitColourGlyph( glyph.Record, size, ox, oy, color );
        return;
    }
    const uint32_t       record = glyph.Record;
    const DenOfIz_Float4 bounds = m_glyphs.GetGlyph( record ).Bounds;
    if ( bounds.Z <= bounds.X || bounds.W <= bounds.Y )
    {
        return;
    }
    const float x0 = ox + bounds.X * size;
    const float x1 = ox + bounds.Z * size;
    const float y0 = oy - bounds.W * size;
    const float y1 = oy - bounds.Y * size;
    const float du = 1.0f / size;
    const float dv = -1.0f / size;
    m_quads.push_back( Quad{ { x0, y0, x1, y1 }, { bounds.X, bounds.W, bounds.Z, bounds.Y }, color, { du, dv }, record, 0 } );
}

// The bitmap is picked for the size the glyph covers on screen at its pen position, colour glyphs ignore the text colour but for its alpha.
void TextRenderer::EmitColourGlyph( const uint32_t source, const float size, const float x, const float y, const DenOfIz_Float4 &color )
{
    auto               channel    = []( const float v ) { return static_cast<uint32_t>( std::lround( std::clamp( v, 0.0f, 1.0f ) * 255.0f ) ); };
    const uint32_t     foreground = channel( color.X ) << 24 | channel( color.Y ) << 16 | channel( color.Z ) << 8 | channel( color.W );
    const ColourGlyph &bitmap     = m_colourGlyphs.Get( source, PixelsPerEm( x, y, size ), foreground );
    if ( bitmap.Width == 0 )
    {
        return;
    }
    const float          k     = size / static_cast<float>( bitmap.Ppem );
    const float          x0    = x + static_cast<float>( bitmap.Left ) * k;
    const float          y0    = y + static_cast<float>( bitmap.Top ) * k;
    const float          x1    = x0 + static_cast<float>( bitmap.Width ) * k;
    const float          y1    = y0 + static_cast<float>( bitmap.Height ) * k;
    const auto           w     = static_cast<float>( bitmap.Width );
    const auto           h     = static_cast<float>( bitmap.Height );
    const DenOfIz_Float4 tint  = { 1.0f, 1.0f, 1.0f, color.W };
    const uint32_t       glyph = bitmap.Offset | ColourGlyphBit;
    m_quads.push_back( Quad{ { x0, y0, x1, y1 }, { 0.0f, 0.0f, w, h }, tint, { 0.0f, 0.0f }, glyph, 0 } );
}

// How many pixels one em at ( x, y ) spans after the projection, the larger of both axes.
float TextRenderer::PixelsPerEm( const float x, const float y, const float size ) const
{
    const DenOfIz_Float4x4 &m       = m_projection;
    auto                    project = [ & ]( const float px, const float py, float &sx, float &sy )
    {
        const float w = px * m._14 + py * m._24 + m._44;
        sx            = ( px * m._11 + py * m._21 + m._41 ) / w * 0.5f * m_viewport.Width;
        sy            = ( px * m._12 + py * m._22 + m._42 ) / w * 0.5f * m_viewport.Height;
    };
    float ox, oy, ax, ay, bx, by;
    project( x, y, ox, oy );
    project( x + size, y, ax, ay );
    project( x, y + size, bx, by );
    return std::max( std::hypot( ax - ox, ay - oy ), std::hypot( bx - ox, by - oy ) );
}

// Every quad uses the same index pattern, so one buffer serves all frames.
void TextRenderer::EnsureQuadCapacity( const uint32_t quads )
{
    if ( quads <= m_quadCapacity )
    {
        return;
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
    m_quadCapacity = GrowCapacity( m_quadCapacity, quads, MinQuadCapacity );
    DenOfIz_BufferDesc desc{ };
    desc.NumBytes  = static_cast<size_t>( m_quadCapacity ) * 6 * sizeof( uint32_t );
    desc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT;
    desc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    desc.DebugName = DENOFIZ_STRING( "Text Indices" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &desc, &m_indexBuffer );
    uint32_t *indices = nullptr;
    DenOfIz_Buffer_MapMemory( m_indexBuffer, reinterpret_cast<void **>( &indices ) );
    for ( uint32_t quad = 0; quad < m_quadCapacity; ++quad )
    {
        const uint32_t base = quad * 4;
        uint32_t      *out  = indices + static_cast<size_t>( quad ) * 6;
        out[ 0 ]            = base;
        out[ 1 ]            = base + 1;
        out[ 2 ]            = base + 2;
        out[ 3 ]            = base;
        out[ 4 ]            = base + 2;
        out[ 5 ]            = base + 3;
    }
    DenOfIz_Buffer_UnmapMemory( m_indexBuffer );
}

// The quads are a structured buffer per frame in flight, bound with the glyph buffers.
void TextRenderer::EnsureFrameCapacity( Frame &frame, const uint32_t index, const uint32_t quads ) const
{
    if ( quads <= frame.QuadCapacity )
    {
        return;
    }
    if ( DENOFIZ_HANDLE_IS_VALID( frame.QuadBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( frame.QuadBuffer );
        DenOfIz_Buffer_Destroy( frame.QuadBuffer );
    }
    frame.QuadCapacity = GrowCapacity( frame.QuadCapacity, quads, MinQuadCapacity );
    DenOfIz_BufferDesc desc{ };
    desc.NumBytes                  = static_cast<size_t>( frame.QuadCapacity ) * sizeof( Quad );
    desc.HeapType                  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    desc.StructureDesc.NumElements = frame.QuadCapacity;
    desc.StructureDesc.Stride      = sizeof( Quad );
    desc.DebugName                 = DENOFIZ_STRING( "Text Quads" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &desc, &frame.QuadBuffer );
    DenOfIz_Buffer_MapMemory( frame.QuadBuffer, reinterpret_cast<void **>( &frame.QuadData ) );
    BindFrame( frame, index );
}

void TextRenderer::BindFrame( Frame &frame, const uint32_t index ) const
{
    DenOfIz_BindBufferDesc uniforms{ };
    uniforms.Resource       = m_uniformBuffer;
    uniforms.ResourceOffset = static_cast<size_t>( index ) * m_alignedUniforms;
    DenOfIz_BindGroup_BeginUpdate( frame.BindGroup );
    DenOfIz_BindGroup_CbvWithDesc( frame.BindGroup, &uniforms );
    m_glyphBuffers.Bind( frame.BindGroup, 0 );
    DenOfIz_BindGroup_SrvBuffer( frame.BindGroup, QuadBinding, frame.QuadBuffer );
    DenOfIz_BindGroup_EndUpdate( frame.BindGroup );
    frame.GlyphGeneration = m_glyphBuffers.Generation( );
}

extern "C"
{
    DenOfIz_TextRenderer DenOfIz_TextRenderer_Create( const DenOfIz_TextRendererDesc *desc )
    {
        if ( desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->LogicalDevice ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return DENOFIZ_TO_HANDLE( new TextRenderer( *desc ) );
    }

    void DenOfIz_TextRenderer_Destroy( const DenOfIz_TextRenderer textRenderer )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            delete TEXT_RENDERER_IMPL( textRenderer );
        }
    }

    void DenOfIz_TextRenderer_SetProjectionMatrix( const DenOfIz_TextRenderer textRenderer, const DenOfIz_Float4x4 *projectionMatrix )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( textRenderer ) && projectionMatrix != nullptr )
        {
            TEXT_RENDERER_IMPL( textRenderer )->SetProjectionMatrix( *projectionMatrix );
        }
    }

    void DenOfIz_TextRenderer_SetViewport( const DenOfIz_TextRenderer textRenderer, const DenOfIz_Viewport *viewport )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( textRenderer ) && viewport != nullptr )
        {
            TEXT_RENDERER_IMPL( textRenderer )->SetViewport( *viewport );
        }
    }

    uint16_t DenOfIz_TextRenderer_AddFont( const DenOfIz_TextRenderer textRenderer, const DenOfIz_Font font, const uint16_t fontId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || !DENOFIZ_HANDLE_IS_VALID( font ) )
        {
            return 0;
        }
        return TEXT_RENDERER_IMPL( textRenderer )->AddFont( FONT_IMPL( font ), fontId );
    }

    DenOfIz_Font DenOfIz_TextRenderer_GetFont( const DenOfIz_TextRenderer textRenderer, const uint16_t fontId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return DENOFIZ_TO_HANDLE( TEXT_RENDERER_IMPL( textRenderer )->GetFont( fontId ) );
    }

    void DenOfIz_TextRenderer_RemoveFont( const DenOfIz_TextRenderer textRenderer, const uint16_t fontId )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            TEXT_RENDERER_IMPL( textRenderer )->RemoveFont( fontId );
        }
    }

    void DenOfIz_TextRenderer_SetFontFallbacks( const DenOfIz_TextRenderer textRenderer, const uint16_t fontId, const DenOfIz_UInt16Array *fallbackIds )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            return;
        }
        std::vector<uint16_t> fallbacks;
        if ( fallbackIds != nullptr && fallbackIds->Elements != nullptr )
        {
            fallbacks.assign( fallbackIds->Elements, fallbackIds->Elements + fallbackIds->NumElements );
        }
        TEXT_RENDERER_IMPL( textRenderer )->SetFallbacks( fontId, std::move( fallbacks ) );
    }

    void DenOfIz_TextRenderer_BeginBatch( const DenOfIz_TextRenderer textRenderer )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            TEXT_RENDERER_IMPL( textRenderer )->BeginBatch( );
        }
    }

    void DenOfIz_TextRenderer_AddText( const DenOfIz_TextRenderer textRenderer, const DenOfIz_TextRenderDesc *textDesc )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( textRenderer ) && textDesc != nullptr )
        {
            TEXT_RENDERER_IMPL( textRenderer )->AddText( *textDesc );
        }
    }

    void DenOfIz_TextRenderer_EndBatch( const DenOfIz_TextRenderer textRenderer, const DenOfIz_CommandList commandList )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( textRenderer ) && DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            TEXT_RENDERER_IMPL( textRenderer )->EndBatch( commandList );
        }
    }

    DenOfIz_Float2 DenOfIz_TextRenderer_MeasureText( const DenOfIz_TextRenderer textRenderer, const DenOfIz_StringView text, const DenOfIz_TextRenderDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || desc == nullptr )
        {
            return DenOfIz_Float2{ 0.0f, 0.0f };
        }
        return TEXT_RENDERER_IMPL( textRenderer )->MeasureText( text, *desc );
    }
}
