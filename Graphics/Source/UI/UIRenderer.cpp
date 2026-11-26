/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/UI/UIRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <string>
#include "DenOfIzGraphics/Backends/Interface/CommandListPool.h"
#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/EmbeddedShaders.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"

using namespace DenOfIz;
using namespace DirectX;

namespace
{
    constexpr uint64_t ShapeCacheAge = 60;

    struct StreamInfo
    {
        uint32_t    Stride;
        uint32_t    MinCapacity;
        const char *Name;
    };

    constexpr std::array<StreamInfo, 4> Streams = { { { sizeof( UIVertex ), 4096, "UI Vertices" },
                                                      { sizeof( UIQuad ), 4096, "UI Quads" },
                                                      { sizeof( uint32_t ), 24576, "UI Indices" },
                                                      { sizeof( UIStateRecord ), 256, "UI States" } } };

    uint32_t GrowCapacity( const uint32_t current, const uint32_t required, const uint32_t minimum )
    {
        uint32_t capacity = std::max( current, minimum );
        while ( capacity < required )
        {
            capacity *= 2;
        }
        return capacity;
    }
} // namespace

UIRenderer::UIRenderer( const UIRendererDesc &desc ) : m_desc( desc ), m_device( desc.LogicalDevice ), m_glyphBuffers( desc.LogicalDevice )
{
    if ( DENOFIZ_HANDLE_IS_VALID( desc.ResourceTracking ) )
    {
        m_resourceTracking = desc.ResourceTracking;
    }
    else
    {
        DenOfIz_ResourceTracking_Create( &m_resourceTracking );
        m_ownsTracking = true;
    }

    m_shaderProgram = EmbeddedShaders::GetUIShaderProgram( );

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

    m_pipeline        = CreatePipeline( desc.DepthFormat );
    m_texturePipeline = desc.DepthFormat == DENOFIZ_FORMAT_UNDEFINED ? m_pipeline : CreatePipeline( DENOFIZ_FORMAT_UNDEFINED );

    DenOfIz_SamplerDesc samplerDesc{ };
    samplerDesc.MagFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MinFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MipmapMode   = DENOFIZ_MIPMAP_MODE_LINEAR;
    samplerDesc.AddressModeU = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeV = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeW = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.CompareOp    = DENOFIZ_COMPARE_OP_ALWAYS;
    DenOfIz_LogicalDevice_CreateSampler( m_device, &samplerDesc, &m_sampler );

    DenOfIz_TextureDesc nullDesc{ };
    nullDesc.Width     = 1;
    nullDesc.Height    = 1;
    nullDesc.Depth     = 1;
    nullDesc.MipLevels = 1;
    nullDesc.ArraySize = 1;
    nullDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    nullDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
    nullDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    nullDesc.DebugName = DENOFIZ_STRING( "UI Null Texture" );
    DenOfIz_LogicalDevice_CreateTexture( m_device, &nullDesc, &m_nullTexture );

    m_alignedUniforms = Utilities::Align( sizeof( Uniforms ), 256 );
    DenOfIz_BufferDesc uniformDesc{ };
    uniformDesc.NumBytes  = m_alignedUniforms * desc.NumFrames;
    uniformDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    uniformDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    uniformDesc.DebugName = DENOFIZ_STRING( "UI Uniforms" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &uniformDesc, &m_uniformBuffer );
    DenOfIz_Buffer_MapMemory( m_uniformBuffer, reinterpret_cast<void **>( &m_uniformData ) );

    DenOfIz_CommandQueueDesc queueDesc{ };
    queueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_device, &queueDesc, &m_queue );
    DenOfIz_CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = m_queue;
    poolDesc.NumCommandLists = desc.NumFrames;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_device, &poolDesc, &m_commandListPool );

    CreateFrameResources( );
}

UIRenderer::~UIRenderer( )
{
    DenOfIz_LogicalDevice_WaitIdle( m_device );
    DestroyCaptureTarget( );
    for ( FrameData &frame : m_frames )
    {
        for ( const FrameStream &stream : frame.Streams )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( stream.Buffer ) )
            {
                DenOfIz_Buffer_UnmapMemory( stream.Buffer );
                DenOfIz_Buffer_Destroy( stream.Buffer );
            }
        }
        for ( const DenOfIz_BindGroup group : { frame.Constants, frame.Textures } )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( group ) )
            {
                DenOfIz_BindGroup_Destroy( group );
            }
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) )
        {
            DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, frame.ColorTarget );
            DenOfIz_Texture_Destroy( frame.ColorTarget );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.Fence ) )
        {
            DenOfIz_Fence_Destroy( frame.Fence );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( frame.Semaphore ) )
        {
            DenOfIz_Semaphore_Destroy( frame.Semaphore );
        }
    }
    if ( m_uniformData )
    {
        DenOfIz_Buffer_UnmapMemory( m_uniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_uniformBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_uniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_sampler ) )
    {
        DenOfIz_Sampler_Destroy( m_sampler );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_nullTexture ) )
    {
        DenOfIz_Texture_Destroy( m_nullTexture );
    }
    if ( m_texturePipeline != m_pipeline && DENOFIZ_HANDLE_IS_VALID( m_texturePipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_texturePipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_pipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_pipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_rootSignature );
    }
    for ( const DenOfIz_BindGroupLayout layout : m_bindGroupLayouts )
    {
        DenOfIz_BindGroupLayout_Destroy( layout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_inputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_inputLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_commandListPool ) )
    {
        DenOfIz_CommandListPool_Destroy( m_commandListPool );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_queue ) )
    {
        DenOfIz_CommandQueue_Destroy( m_queue );
    }
    if ( m_ownsTracking )
    {
        DenOfIz_ResourceTracking_Destroy( m_resourceTracking );
    }
}

DenOfIz_Pipeline UIRenderer::CreatePipeline( const DenOfIz_Format depthFormat ) const
{
    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.RootSignature                         = m_rootSignature;
    pipelineDesc.InputLayout                           = m_inputLayout;
    pipelineDesc.ShaderProgram                         = m_shaderProgram;
    pipelineDesc.BindPoint                             = DENOFIZ_BIND_POINT_GRAPHICS;
    pipelineDesc.Graphics.PrimitiveTopology            = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.CullMode                     = DENOFIZ_CULL_MODE_NONE;
    pipelineDesc.Graphics.FillMode                     = DENOFIZ_FILL_MODE_SOLID;
    pipelineDesc.Graphics.DepthTest.Enable             = false;
    pipelineDesc.Graphics.DepthTest.Write              = false;
    pipelineDesc.Graphics.DepthTest.CompareOp          = DENOFIZ_COMPARE_OP_ALWAYS;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = depthFormat;

    DenOfIz_RenderTargetDesc renderTarget{ };
    renderTarget.Format                      = m_desc.RenderTargetFormat;
    renderTarget.Blend.Enable                = true;
    renderTarget.Blend.SrcBlend              = DENOFIZ_BLEND_SRC_ALPHA;
    renderTarget.Blend.DstBlend              = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOp               = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.SrcBlendAlpha         = DENOFIZ_BLEND_ONE;
    renderTarget.Blend.DstBlendAlpha         = DENOFIZ_BLEND_INV_SRC_ALPHA;
    renderTarget.Blend.BlendOpAlpha          = DENOFIZ_BLEND_OP_ADD;
    renderTarget.Blend.RenderTargetWriteMask = 0x0F;
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    DenOfIz_Pipeline pipeline = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreatePipeline( m_device, &pipelineDesc, &pipeline );
    return pipeline;
}

void UIRenderer::CreateFrameResources( )
{
    m_frames.resize( m_desc.NumFrames );
    DenOfIz_CommandListArray commandLists{ };
    DenOfIz_CommandListPool_GetCommandLists( m_commandListPool, &commandLists );
    for ( uint32_t i = 0; i < m_desc.NumFrames; ++i )
    {
        FrameData &frame  = m_frames[ i ];
        frame.CommandList = i < commandLists.NumElements ? commandLists.Elements[ i ] : DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice_CreateFence( m_device, &frame.Fence );
        DenOfIz_LogicalDevice_CreateSemaphore( m_device, &frame.Semaphore );

        DenOfIz_BindGroupDesc constantsDesc{ };
        constantsDesc.Layout = m_bindGroupLayouts[ 1 ];
        DenOfIz_LogicalDevice_CreateBindGroup( m_device, &constantsDesc, &frame.Constants );

        DenOfIz_BindGroupDesc texturesDesc{ };
        texturesDesc.Layout = m_bindGroupLayouts[ 0 ];
        DenOfIz_LogicalDevice_CreateBindGroup( m_device, &texturesDesc, &frame.Textures );
        for ( uint32_t stream = 0; stream < NumStreams; ++stream )
        {
            Reserve( frame, static_cast<Stream>( stream ), Streams[ stream ].MinCapacity, 0 );
        }
    }
}

void UIRenderer::Reserve( FrameData &frame, const Stream stream, const uint32_t count, const uint32_t keep ) const
{
    FrameStream &target = frame.Streams[ stream ];
    if ( count <= target.Capacity )
    {
        return;
    }
    const StreamInfo  &info     = Streams[ stream ];
    const uint32_t     capacity = GrowCapacity( target.Capacity, count, info.MinCapacity );
    DenOfIz_BufferDesc desc{ };
    desc.NumBytes  = static_cast<uint64_t>( capacity ) * info.Stride;
    desc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    desc.DebugName = DENOFIZ_STRING( info.Name );
    if ( stream == Indices )
    {
        desc.Usage = DENOFIZ_BUFFER_USAGE_INDEX_BIT;
    }
    else
    {
        desc.StructureDesc.NumElements = capacity;
        desc.StructureDesc.Stride      = info.Stride;
        frame.ConstantsStale           = true;
    }
    DenOfIz_Buffer buffer = DENOFIZ_NULL_HANDLE;
    uint8_t       *data   = nullptr;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &desc, &buffer );
    DenOfIz_Buffer_MapMemory( buffer, reinterpret_cast<void **>( &data ) );
    if ( DENOFIZ_HANDLE_IS_VALID( target.Buffer ) )
    {
        std::memcpy( data, target.Data, static_cast<size_t>( keep ) * info.Stride );
        DenOfIz_Buffer_UnmapMemory( target.Buffer );
        DenOfIz_Buffer_Destroy( target.Buffer );
    }
    target = FrameStream{ buffer, data, capacity };
}

template <typename T>
T *UIRenderer::StreamData( const Stream stream, const uint32_t first ) const
{
    return reinterpret_cast<T *>( m_target->Streams[ stream ].Data ) + first;
}

void UIRenderer::BindFrameConstants( const uint32_t frameIndex )
{
    FrameData             &frame = m_frames[ frameIndex ];
    DenOfIz_BindBufferDesc bindUniforms{ };
    bindUniforms.Resource       = m_uniformBuffer;
    bindUniforms.ResourceOffset = frameIndex * m_alignedUniforms;
    DenOfIz_BindGroup_BeginUpdate( frame.Constants );
    DenOfIz_BindGroup_CbvWithDesc( frame.Constants, &bindUniforms );
    DenOfIz_BindGroup_SrvBuffer( frame.Constants, 0, frame.Streams[ States ].Buffer );
    m_glyphBuffers.Bind( frame.Constants, 1 );
    DenOfIz_BindGroup_SrvBuffer( frame.Constants, 5, frame.Streams[ Vertices ].Buffer );
    DenOfIz_BindGroup_SrvBuffer( frame.Constants, 6, frame.Streams[ Quads ].Buffer );
    DenOfIz_BindGroup_EndUpdate( frame.Constants );
    frame.GlyphGeneration = m_glyphBuffers.Generation( );
    frame.ConstantsStale  = false;
}

uint32_t UIRenderer::AllocateRecord( )
{
    if ( m_recordCount >= m_target->Streams[ States ].Capacity )
    {
        Reserve( *m_target, States, m_recordCount + 1, m_recordCount );
    }
    return m_recordCount++;
}

void UIRenderer::PushState( const DrawState &state )
{
    DrawState next = state;
    next.Record    = 0;
    if ( next.ClipCount > 0 || !UIMath::IsIdentityLayer( next.Layer ) )
    {
        next.Record                                       = AllocateRecord( );
        const UILayerCommand &layer                       = next.Layer;
        *StreamData<UIStateRecord>( States, next.Record ) = UIStateRecord{ { layer.BiasR, layer.BiasG, layer.BiasB, layer.Scale },
                                                                           { layer.Opacity, static_cast<float>( next.ClipCount ), 0.0f, 0.0f },
                                                                           { static_cast<float>( next.Clips[ 0 ] ), static_cast<float>( next.Clips[ 1 ] ),
                                                                             static_cast<float>( next.Clips[ 2 ] ), static_cast<float>( next.Clips[ 3 ] ) } };
    }
    m_stateStack.push_back( next );
    m_stateBits = next.Record << UITextureSlots::Bits;
}

UIVertex *UIRenderer::AllocateVertices( const uint32_t count )
{
    if ( m_vertexCount + count > UIQuadIndexBit )
    {
        if ( !m_geometryExhaustedReported )
        {
            spdlog::error( "UI: more than {} vertices or {} quads in one frame, the rest are not drawn", UIQuadIndexBit, UIMaxQuads );
            m_geometryExhaustedReported = true;
        }
        return nullptr;
    }
    if ( m_vertexCount + count > m_target->Streams[ Vertices ].Capacity )
    {
        Reserve( *m_target, Vertices, m_vertexCount + count, m_vertexCount );
    }
    UIVertex *vertices = StreamData<UIVertex>( Vertices, m_vertexCount );
    m_vertexCount += count;
    return vertices;
}

UIQuad *UIRenderer::AllocateQuad( )
{
    if ( m_quadCount >= UIMaxQuads )
    {
        if ( !m_geometryExhaustedReported )
        {
            spdlog::error( "UI: more than {} vertices or {} quads in one frame, the rest are not drawn", UIQuadIndexBit, UIMaxQuads );
            m_geometryExhaustedReported = true;
        }
        return nullptr;
    }
    if ( m_quadCount >= m_target->Streams[ Quads ].Capacity )
    {
        Reserve( *m_target, Quads, m_quadCount + 1, m_quadCount );
    }
    const uint32_t base    = UIQuadIndexBit | m_quadCount * 4;
    uint32_t      *indices = AllocateIndices( 6 );
    indices[ 0 ]           = base;
    indices[ 1 ]           = base + 1;
    indices[ 2 ]           = base + 2;
    indices[ 3 ]           = base;
    indices[ 4 ]           = base + 2;
    indices[ 5 ]           = base + 3;
    return StreamData<UIQuad>( Quads, m_quadCount++ );
}

uint32_t *UIRenderer::AllocateIndices( const uint32_t count )
{
    if ( m_indexCount + count > m_target->Streams[ Indices ].Capacity )
    {
        Reserve( *m_target, Indices, m_indexCount + count, m_indexCount );
    }
    uint32_t *indices = StreamData<uint32_t>( Indices, m_indexCount );
    m_indexCount += count;
    m_batches.back( ).IndexCount += count;
    return indices;
}

void UIRenderer::EnsureColorTarget( FrameData &frame, const uint32_t width, const uint32_t height ) const
{
    if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) && frame.TargetWidth == width && frame.TargetHeight == height )
    {
        return;
    }
    if ( DENOFIZ_HANDLE_IS_VALID( frame.ColorTarget ) )
    {
        DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, frame.ColorTarget );
        DenOfIz_Texture_Destroy( frame.ColorTarget );
    }
    DenOfIz_TextureDesc desc{ };
    desc.Width     = width;
    desc.Height    = height;
    desc.Depth     = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format    = m_desc.RenderTargetFormat;
    desc.Usage     = DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT | DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
    desc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    desc.DebugName = DENOFIZ_STRING( "UI Color Target" );
    DenOfIz_LogicalDevice_CreateTexture( m_device, &desc, &frame.ColorTarget );
    DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, frame.ColorTarget, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    frame.TargetWidth  = width;
    frame.TargetHeight = height;
}

uint32_t UIRenderer::SlotOf( const DenOfIz_Texture texture )
{
    for ( uint32_t slot = UITextureSlots::FirstImage; slot < m_nextImageSlot; ++slot )
    {
        if ( m_slots[ slot ] == texture )
        {
            return slot;
        }
    }
    if ( m_nextImageSlot >= UITextureSlots::Count )
    {
        if ( !m_slotsExhaustedReported )
        {
            spdlog::error( "UI: more than {} distinct images in one frame, the rest are not drawn", UITextureSlots::Count - UITextureSlots::FirstImage );
            m_slotsExhaustedReported = true;
        }
        return UINT32_MAX;
    }
    m_slots[ m_nextImageSlot ] = texture;
    return m_nextImageSlot++;
}

DenOfIz_Float4 UIRenderer::ShadeColor( const DenOfIz_UIColor &color ) const
{
    constexpr float inv = 1.0f / 255.0f;
    return DenOfIz_Float4{ color.R * inv, color.G * inv, color.B * inv, color.A * inv };
}

void UIRenderer::SetScissor( const Scissor &scissor )
{
    Batch &batch = m_batches.back( );
    if ( batch.IndexCount == 0 )
    {
        batch.Clip       = scissor;
        batch.FirstIndex = m_indexCount;
        return;
    }
    Batch next;
    next.FirstIndex = m_indexCount;
    next.Clip       = scissor;
    m_batches.push_back( next );
}

void UIRenderer::EmitQuad( const float x0, const float y0, const float x1, const float y1, const float u0, const float v0, const float u1, const float v1,
                           const DenOfIz_UIColor &color, const uint32_t slot )
{
    if ( UIQuad *quad = AllocateQuad( ) )
    {
        *quad = UIQuad{ { x0, y0, x1, y1 }, { u0, v0, u1, v1 }, ShadeColor( color ), slot | m_stateBits, 0, {} };
    }
}

void UIRenderer::EmitShape( const UIShape &shape, const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, const uint32_t slot, const DenOfIz_Float4 *uv )
{
    const uint32_t       index    = slot | m_stateBits;
    const uint32_t       base     = m_vertexCount;
    const float          invW     = rect.Width > 0.0f ? 1.0f / rect.Width : 0.0f;
    const float          invH     = rect.Height > 0.0f ? 1.0f / rect.Height : 0.0f;
    const DenOfIz_Float4 full     = ShadeColor( color );
    const auto           count    = static_cast<uint32_t>( shape.Vertices.size( ) );
    UIVertex            *vertices = AllocateVertices( count );
    if ( vertices == nullptr )
    {
        return;
    }
    for ( uint32_t i = 0; i < count; ++i )
    {
        const UIShapeVertex &v = shape.Vertices[ i ];
        DenOfIz_Float2       texCoord{ 0.0f, 0.0f };
        if ( uv )
        {
            const float u = std::clamp( v.X * invW, 0.0f, 1.0f );
            const float t = std::clamp( v.Y * invH, 0.0f, 1.0f );
            texCoord      = DenOfIz_Float2{ uv->X + ( uv->Z - uv->X ) * u, uv->Y + ( uv->W - uv->Y ) * t };
        }
        vertices[ i ] = UIVertex{ { full.X, full.Y, full.Z, full.W * v.Coverage }, { rect.X + v.X, rect.Y + v.Y, 0.0f }, index, texCoord, 0, 0 };
    }
    const auto indexCount = static_cast<uint32_t>( shape.Indices.size( ) );
    uint32_t  *indices    = AllocateIndices( indexCount );
    for ( uint32_t i = 0; i < indexCount; ++i )
    {
        indices[ i ] = base + shape.Indices[ i ];
    }
}

void UIRenderer::EmitText( const UICommand &command )
{
    const UITextLayout &layout       = *command.Text.Layout;
    ColourGlyphStore   &colourGlyphs = m_desc.Text->ColourGlyphs( );
    const auto         &quads        = layout.GetQuads( m_desc.Text->Glyphs( ), colourGlyphs );
    if ( quads.empty( ) )
    {
        return;
    }
    const float          s          = command.Text.Scale;
    const DenOfIz_Float4 color      = ShadeColor( command.Text.Color );
    const uint32_t       index      = UITextureSlots::Glyph | m_stateBits;
    const float          pixel      = 1.0f / m_pixelsPerPoint;
    const float          clipTop    = m_clipPoints.Y;
    const float          clipBottom = m_clipPoints.Y + m_clipPoints.Height;
    auto pack = []( const DenOfIz_UIColor &c ) { return static_cast<uint32_t>( c.R ) << 24 | static_cast<uint32_t>( c.G ) << 16 | static_cast<uint32_t>( c.B ) << 8 | c.A; };
    const uint32_t       textForeground = pack( command.Text.Color );
    const DenOfIz_Float4 textTint       = { 1.0f, 1.0f, 1.0f, color.W };
    const uint32_t       colourIndex    = UITextureSlots::ColourGlyph | m_stateBits;
    const auto          &styles         = layout.Run->Styles;
    uint32_t             style          = UINT32_MAX;
    DenOfIz_Float4       glyphColor     = color;
    DenOfIz_Float4       tint           = textTint;
    uint32_t             foreground     = textForeground;

    auto addQuad = [ this ]( const UIQuad &quad )
    {
        if ( UIQuad *target = AllocateQuad( ) )
        {
            *target = quad;
        }
    };

    uint32_t line    = UINT32_MAX;
    float    lineX   = 0.0f;
    float    lineY   = 0.0f;
    bool     visible = true;
    for ( const UIGlyphQuad &quad : quads )
    {
        if ( quad.Line != line )
        {
            line                        = quad.Line;
            const UITextLineLayout &l   = layout.Lines[ line ];
            const float             top = command.Rect.Y + l.Top * s;
            if ( top > clipBottom )
            {
                break;
            }
            visible = top + l.Height * s >= clipTop;
            lineX   = command.Rect.X + layout.AlignOffset( line, command.Text.BoxWidth, command.Text.Align ) * s;
            lineY   = std::round( ( command.Rect.Y + l.Baseline * s ) * m_pixelsPerPoint ) / m_pixelsPerPoint;
        }
        if ( !visible )
        {
            continue;
        }
        if ( quad.Style != style )
        {
            style = quad.Style;
            if ( style < styles.size( ) )
            {
                glyphColor = ShadeColor( styles[ style ].Color );
                tint       = { 1.0f, 1.0f, 1.0f, glyphColor.W };
                foreground = pack( styles[ style ].Color );
            }
            else
            {
                glyphColor = color;
                tint       = textTint;
                foreground = textForeground;
            }
        }
        const float pixelsPerEm = quad.Em * s * m_pixelsPerPoint;
        if ( quad.Colour )
        {
            const ColourGlyph &bitmap = colourGlyphs.Get( quad.Glyph, pixelsPerEm, foreground );
            if ( bitmap.Width == 0 )
            {
                continue;
            }
            const float k  = pixelsPerEm / static_cast<float>( bitmap.Ppem ) * pixel;
            const float x0 = std::round( ( lineX + quad.X0 * s + static_cast<float>( bitmap.Left ) * k ) * m_pixelsPerPoint ) * pixel;
            const float y0 = std::round( ( lineY + quad.Y0 * s + static_cast<float>( bitmap.Top ) * k ) * m_pixelsPerPoint ) * pixel;
            const float x1 = x0 + static_cast<float>( bitmap.Width ) * k;
            const float y1 = y0 + static_cast<float>( bitmap.Height ) * k;
            const auto  w  = static_cast<float>( bitmap.Width );
            const auto  h  = static_cast<float>( bitmap.Height );
            addQuad( UIQuad{ { x0, y0, x1, y1 }, { 0.0f, 0.0f, w, h }, tint, colourIndex, bitmap.Offset, {} } );
            continue;
        }
        const float x0 = lineX + quad.X0 * s;
        const float x1 = lineX + quad.X1 * s;
        const float y0 = lineY + quad.Y0 * s;
        const float y1 = lineY + quad.Y1 * s;
        if ( x1 <= x0 || y1 <= y0 )
        {
            continue;
        }
        const float du = ( quad.U1 - quad.U0 ) / ( x1 - x0 ) * pixel;
        const float dv = ( quad.V1 - quad.V0 ) / ( y1 - y0 ) * pixel;
        const float u0 = quad.U0 - du;
        const float u1 = quad.U1 + du;
        const float v0 = quad.V0 - dv;
        const float v1 = quad.V1 + dv;
        addQuad( UIQuad{ { x0 - pixel, y0 - pixel, x1 + pixel, y1 + pixel }, { u0, v0, u1, v1 }, glyphColor, index, quad.Glyph, {} } );
    }
}

void UIRenderer::Build( const std::vector<UICommand> &commands, const uint32_t frameIndex, const float pixelWidth, const float pixelHeight, const float pixelsPerPoint )
{
    ++m_frameCounter;
    m_pixelsPerPoint = pixelsPerPoint;
    m_target         = &m_frames[ frameIndex ];
    m_vertexCount    = 0;
    m_quadCount      = 0;
    m_indexCount     = 0;
    m_recordCount    = 1;
    m_stateBits      = 0;
    m_desc.Text->ColourGlyphs( ).BeginFrame( );
    *StreamData<UIStateRecord>( States, 0 ) = UIStateRecord{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } };
    m_batches.clear( );
    m_clipStack.clear( );
    m_stateStack.clear( );
    m_stateStack.push_back( DrawState{ } );
    m_slots.fill( DENOFIZ_NULL_HANDLE );
    m_nextImageSlot = UITextureSlots::FirstImage;

    const Scissor viewport{ 0, 0, static_cast<int32_t>( pixelWidth ), static_cast<int32_t>( pixelHeight ) };
    m_clipStack.push_back( viewport );
    m_clipPoints = DenOfIz_UIRect{ 0.0f, 0.0f, pixelWidth / pixelsPerPoint, pixelHeight / pixelsPerPoint };
    m_batches.push_back( Batch{ 0, 0, viewport } );

    const float feather = 1.0f / pixelsPerPoint;
    for ( const UICommand &command : commands )
    {
        switch ( command.Type )
        {
        case UICommandType::PushClip:
        {
            const Scissor &parent = m_clipStack.back( );
            const auto     left   = static_cast<float>( parent.X );
            const auto     top    = static_cast<float>( parent.Y );
            const auto     right  = static_cast<float>( parent.X + parent.Width );
            const auto     bottom = static_cast<float>( parent.Y + parent.Height );
            const auto     x0     = static_cast<int32_t>( std::clamp( std::floor( command.Rect.X * pixelsPerPoint ), left, right ) );
            const auto     y0     = static_cast<int32_t>( std::clamp( std::floor( command.Rect.Y * pixelsPerPoint ), top, bottom ) );
            const auto     x1     = static_cast<int32_t>( std::clamp( std::ceil( ( command.Rect.X + command.Rect.Width ) * pixelsPerPoint ), left, right ) );
            const auto     y1     = static_cast<int32_t>( std::clamp( std::ceil( ( command.Rect.Y + command.Rect.Height ) * pixelsPerPoint ), top, bottom ) );
            const Scissor  clip{ x0, y0, std::max( 0, x1 - x0 ), std::max( 0, y1 - y0 ) };
            m_clipStack.push_back( clip );
            m_clipPoints = DenOfIz_UIRect{ clip.X / pixelsPerPoint, clip.Y / pixelsPerPoint, clip.Width / pixelsPerPoint, clip.Height / pixelsPerPoint };
            SetScissor( clip );
            if ( !command.Clip.Rounded )
            {
                const DrawState current = m_stateStack.back( );
                m_stateStack.push_back( current );
                break;
            }
            const uint32_t          record = AllocateRecord( );
            const DenOfIz_UICorners radius = UIMath::ScaleCorners( command.Clip.Radius, pixelsPerPoint );
            *StreamData<UIStateRecord>( States, record ) =
                UIStateRecord{ { command.Rect.X * pixelsPerPoint, command.Rect.Y * pixelsPerPoint, ( command.Rect.X + command.Rect.Width ) * pixelsPerPoint,
                                 ( command.Rect.Y + command.Rect.Height ) * pixelsPerPoint },
                               { radius.TopLeft, radius.TopRight, radius.BottomRight, radius.BottomLeft },
                               { 0.0f, 0.0f, 0.0f, 0.0f } };
            DrawState next = m_stateStack.back( );
            for ( uint32_t i = std::min( next.ClipCount, UIMaxRoundedClips - 1 ); i > 0; --i )
            {
                next.Clips[ i ] = next.Clips[ i - 1 ];
            }
            next.Clips[ 0 ] = record;
            next.ClipCount  = std::min( next.ClipCount + 1, UIMaxRoundedClips );
            PushState( next );
            break;
        }
        case UICommandType::PopClip:
        {
            if ( m_clipStack.size( ) > 1 )
            {
                m_clipStack.pop_back( );
            }
            if ( m_stateStack.size( ) > 1 )
            {
                m_stateStack.pop_back( );
            }
            m_stateBits         = m_stateStack.back( ).Record << UITextureSlots::Bits;
            const Scissor &clip = m_clipStack.back( );
            m_clipPoints        = DenOfIz_UIRect{ clip.X / pixelsPerPoint, clip.Y / pixelsPerPoint, clip.Width / pixelsPerPoint, clip.Height / pixelsPerPoint };
            SetScissor( clip );
            break;
        }
        case UICommandType::PushLayer:
        {
            DrawState next = m_stateStack.back( );
            next.Layer     = UIMath::ComposeLayer( next.Layer, command.Layer );
            PushState( next );
            break;
        }
        case UICommandType::PopLayer:
            if ( m_stateStack.size( ) > 1 )
            {
                m_stateStack.pop_back( );
            }
            m_stateBits = m_stateStack.back( ).Record << UITextureSlots::Bits;
            break;
        case UICommandType::Fill:
            if ( !UIMath::Intersects( command.Rect, m_clipPoints ) )
            {
                break;
            }
            if ( UIMath::HasCorners( command.Fill.Radius ) )
            {
                EmitShape( m_shapes.GetFill( command.Rect.Width, command.Rect.Height, command.Fill.Radius, feather, m_frameCounter ), command.Rect, command.Fill.Color, 0, nullptr );
            }
            else
            {
                EmitQuad( command.Rect.X, command.Rect.Y, command.Rect.X + command.Rect.Width, command.Rect.Y + command.Rect.Height, 0.0f, 0.0f, 0.0f, 0.0f, command.Fill.Color, 0 );
            }
            break;
        case UICommandType::Border:
            if ( UIMath::Intersects( command.Rect, m_clipPoints ) )
            {
                EmitShape( m_shapes.GetBorder( command.Rect.Width, command.Rect.Height, command.Border.Widths, command.Border.Radius, feather, m_frameCounter ), command.Rect,
                           command.Border.Color, 0, nullptr );
            }
            break;
        case UICommandType::Image:
        {
            if ( !UIMath::Intersects( command.Rect, m_clipPoints ) )
            {
                break;
            }
            const uint32_t slot = SlotOf( command.Image.Texture );
            if ( slot == UINT32_MAX )
            {
                break;
            }
            const DenOfIz_Float4 &uv = command.Image.UV;
            if ( UIMath::HasCorners( command.Image.Radius ) )
            {
                EmitShape( m_shapes.GetFill( command.Rect.Width, command.Rect.Height, command.Image.Radius, feather, m_frameCounter ), command.Rect, command.Image.Tint, slot, &uv );
            }
            else
            {
                EmitQuad( command.Rect.X, command.Rect.Y, command.Rect.X + command.Rect.Width, command.Rect.Y + command.Rect.Height, uv.X, uv.Y, uv.Z, uv.W, command.Image.Tint, slot );
            }
            break;
        }
        case UICommandType::Text:
            EmitText( command );
            break;
        }
    }

    if ( m_frameCounter % 30 == 0 )
    {
        m_shapes.Evict( m_frameCounter, ShapeCacheAge );
    }
    m_glyphBuffers.Write( m_desc.Text->Glyphs( ), m_desc.Text->ColourGlyphs( ) );
}

void UIRenderer::Upload( const uint32_t frameIndex, const float pixelWidth, const float pixelHeight )
{
    FrameData &frame = m_frames[ frameIndex ];
    const float    width  = pixelWidth / m_pixelsPerPoint;
    const float    height = pixelHeight / m_pixelsPerPoint;
    const XMMATRIX ortho  = XMMatrixOrthographicOffCenterLH( 0.0f, width, height, 0.0f, 0.0f, 1.0f );
    XMFLOAT4X4     projection;
    XMStoreFloat4x4( &projection, ortho );

    Uniforms uniforms{ };
    uniforms.Projection = InteropMathConverter::Float_4x4FromXMFLOAT4X4( projection );
    uniforms.ScreenSize = DenOfIz_Float4{ pixelWidth, pixelHeight, m_pixelsPerPoint, 0.0f };
    std::memcpy( m_uniformData + frameIndex * m_alignedUniforms, &uniforms, sizeof( Uniforms ) );
    if ( frame.ConstantsStale || frame.GlyphGeneration != m_glyphBuffers.Generation( ) )
    {
        BindFrameConstants( frameIndex );
    }

    if ( !frame.BoundValid || frame.Bound != m_slots )
    {
        std::array<DenOfIz_Texture, UITextureSlots::Count> textures{ };
        for ( uint32_t i = 0; i < UITextureSlots::Count; ++i )
        {
            textures[ i ] = DENOFIZ_HANDLE_IS_VALID( m_slots[ i ] ) ? m_slots[ i ] : m_nullTexture;
        }
        DenOfIz_TextureArray array{ };
        array.Elements    = textures.data( );
        array.NumElements = UITextureSlots::Count;
        DenOfIz_BindGroup_BeginUpdate( frame.Textures );
        DenOfIz_BindGroup_SrvArray( frame.Textures, 0, &array );
        DenOfIz_BindGroup_Sampler( frame.Textures, 0, m_sampler );
        DenOfIz_BindGroup_EndUpdate( frame.Textures );
        frame.Bound      = m_slots;
        frame.BoundValid = true;
    }
}

void UIRenderer::Record( const uint32_t frameIndex, const DenOfIz_CommandList commandList, const DenOfIz_Pipeline pipeline, const float pixelWidth, const float pixelHeight )
{
    if ( m_indexCount == 0 )
    {
        return;
    }
    const FrameData &frame = m_frames[ frameIndex ];
    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, pixelWidth, pixelHeight );
    DenOfIz_CommandList_BindPipeline( commandList, pipeline );
    DenOfIz_CommandList_BindGroup( commandList, frame.Constants );
    DenOfIz_CommandList_BindGroup( commandList, frame.Textures );
    DenOfIz_CommandList_BindIndexBuffer( commandList, frame.Streams[ Indices ].Buffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );

    for ( const Batch &batch : m_batches )
    {
        if ( batch.IndexCount == 0 || batch.Clip.Width <= 0 || batch.Clip.Height <= 0 )
        {
            continue;
        }
        DenOfIz_CommandList_BindScissorRect( commandList, static_cast<float>( batch.Clip.X ), static_cast<float>( batch.Clip.Y ), static_cast<float>( batch.Clip.Width ),
                                             static_cast<float>( batch.Clip.Height ) );
        DenOfIz_CommandList_DrawIndexed( commandList, batch.IndexCount, 1, batch.FirstIndex, 0, 0 );
    }
}

void UIRenderer::Render( const std::vector<UICommand> &commands, const uint32_t frameIndex, const DenOfIz_CommandList commandList, const float pixelWidth, const float pixelHeight,
                         const float pixelsPerPoint )
{
    if ( frameIndex >= m_frames.size( ) )
    {
        spdlog::error( "UI: frame index {} is out of range, NumFrames is {}", frameIndex, m_frames.size( ) );
        return;
    }
    Build( commands, frameIndex, pixelWidth, pixelHeight, pixelsPerPoint );
    Upload( frameIndex, pixelWidth, pixelHeight );
    Record( frameIndex, commandList, m_pipeline, pixelWidth, pixelHeight );
}

void UIRenderer::RenderToTexture( const std::vector<UICommand> &commands, const uint32_t frameIndex, const uint32_t pixelWidth, const uint32_t pixelHeight, const float pixelsPerPoint,
                                  const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_UIRenderResult *outResult )
{
    if ( frameIndex >= m_frames.size( ) )
    {
        spdlog::error( "UI: frame index {} is out of range, NumFrames is {}", frameIndex, m_frames.size( ) );
        return;
    }
    FrameData &frame = m_frames[ frameIndex ];
    if ( frame.Submitted )
    {
        DenOfIz_Fence_Wait( frame.Fence );
    }
    EnsureColorTarget( frame, std::max( 1u, pixelWidth ), std::max( 1u, pixelHeight ) );

    const auto width  = static_cast<float>( frame.TargetWidth );
    const auto height = static_cast<float>( frame.TargetHeight );
    Build( commands, frameIndex, width, height, pixelsPerPoint );
    Upload( frameIndex, width, height );

    const DenOfIz_CommandList commandList = frame.CommandList;
    DenOfIz_CommandList_Begin( commandList );
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, frame.ColorTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc colorAttachment{ };
    colorAttachment.Resource   = frame.ColorTarget;
    colorAttachment.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    colorAttachment.StoreOp    = DENOFIZ_STORE_OP_STORE;
    colorAttachment.ClearColor = DenOfIz_Float4{ 0.0f, 0.0f, 0.0f, 0.0f };

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &colorAttachment;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;
    renderingDesc.RenderAreaWidth           = width;
    renderingDesc.RenderAreaHeight          = height;
    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, width, height );
    Record( frameIndex, commandList, m_texturePipeline, width, height );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, frame.ColorTarget, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_CommandList_End( commandList );

    DenOfIz_ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements        = &frame.CommandList;
    executeDesc.CommandLists.NumElements     = 1;
    executeDesc.Signal                       = frame.Fence;
    executeDesc.SignalSemaphores.Elements    = &frame.Semaphore;
    executeDesc.SignalSemaphores.NumElements = 1;
    if ( waitSemaphores )
    {
        executeDesc.WaitSemaphores = *waitSemaphores;
    }
    DenOfIz_CommandQueue_ExecuteCommandLists( m_queue, &executeDesc );
    frame.Submitted = true;

    if ( outResult )
    {
        outResult->Texture   = frame.ColorTarget;
        outResult->Semaphore = frame.Semaphore;
    }
}

void UIRenderer::EnsureCaptureTarget( const uint32_t width, const uint32_t height )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_captureTarget ) && m_captureWidth == width && m_captureHeight == height )
    {
        return;
    }
    DestroyCaptureTarget( );
    DenOfIz_TextureDesc desc{ };
    desc.Width     = width;
    desc.Height    = height;
    desc.Depth     = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format    = m_desc.RenderTargetFormat;
    desc.Usage     = DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT | DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    desc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    desc.DebugName = DENOFIZ_STRING( "UI Capture Target" );
    DenOfIz_LogicalDevice_CreateTexture( m_device, &desc, &m_captureTarget );
    DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_captureTarget, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_BufferDesc readbackDesc{ };
    readbackDesc.NumBytes  = static_cast<size_t>( width ) * height * 4;
    readbackDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    readbackDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU_CPU;
    readbackDesc.DebugName = DENOFIZ_STRING( "UI Capture Readback" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &readbackDesc, &m_captureReadback );
    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_captureReadback, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    m_captureWidth  = width;
    m_captureHeight = height;
}

void UIRenderer::DestroyCaptureTarget( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_captureTarget ) )
    {
        DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, m_captureTarget );
        DenOfIz_Texture_Destroy( m_captureTarget );
        m_captureTarget = DENOFIZ_NULL_HANDLE;
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_captureReadback ) )
    {
        DenOfIz_ResourceTracking_UntrackBuffer( m_resourceTracking, m_captureReadback );
        DenOfIz_Buffer_Destroy( m_captureReadback );
        m_captureReadback = DENOFIZ_NULL_HANDLE;
    }
    m_captureWidth = m_captureHeight = 0;
}

bool UIRenderer::Capture( const std::vector<UICommand> &commands, const uint32_t frameIndex, const uint32_t pixelWidth, const uint32_t pixelHeight, const float pixelsPerPoint,
                          const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_ByteArray *outPixels )
{
    const DenOfIz_Format format = m_desc.RenderTargetFormat;
    const bool           bgra   = format == DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    const size_t         bytes  = static_cast<size_t>( pixelWidth ) * pixelHeight * 4;
    if ( frameIndex >= m_frames.size( ) )
    {
        spdlog::error( "UI: frame index {} is out of range, NumFrames is {}", frameIndex, m_frames.size( ) );
        return false;
    }
    if ( !bgra && format != DENOFIZ_FORMAT_R8G8B8A8_UNORM && format != DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB )
    {
        spdlog::error( "UI: Capture needs an 8 bit RGBA or BGRA render target format" );
        return false;
    }
    if ( outPixels == nullptr || outPixels->Elements == nullptr || outPixels->NumElements < bytes || bytes == 0 )
    {
        spdlog::error( "UI: Capture needs {} bytes for {}x{} pixels", bytes, pixelWidth, pixelHeight );
        return false;
    }

    const uint32_t paddedWidth = ( pixelWidth + 63u ) & ~63u;
    DenOfIz_LogicalDevice_WaitIdle( m_device );
    EnsureCaptureTarget( paddedWidth, pixelHeight );
    FrameData &frame  = m_frames[ frameIndex ];
    const auto width  = static_cast<float>( paddedWidth );
    const auto height = static_cast<float>( pixelHeight );
    Build( commands, frameIndex, width, height, pixelsPerPoint );
    Upload( frameIndex, width, height );

    const DenOfIz_CommandList commandList = frame.CommandList;
    DenOfIz_CommandList_Begin( commandList );
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, m_captureTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc colorAttachment{ };
    colorAttachment.Resource   = m_captureTarget;
    colorAttachment.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    colorAttachment.StoreOp    = DENOFIZ_STORE_OP_STORE;
    colorAttachment.ClearColor = DenOfIz_Float4{ 0.0f, 0.0f, 0.0f, 0.0f };

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &colorAttachment;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;
    renderingDesc.RenderAreaWidth           = width;
    renderingDesc.RenderAreaHeight          = height;
    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, width, height );
    Record( frameIndex, commandList, m_texturePipeline, width, height );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, m_captureTarget, DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_ResourceTracking_TransitionBuffer( m_resourceTracking, commandList, m_captureReadback, DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_CopyTextureToBufferDesc copy{ };
    copy.DstBuffer  = m_captureReadback;
    copy.SrcTexture = m_captureTarget;
    copy.Format     = format;
    copy.RowPitch   = paddedWidth * 4;
    copy.NumRows    = pixelHeight;
    DenOfIz_CommandList_CopyTextureToBuffer( commandList, &copy );
    DenOfIz_CommandList_End( commandList );

    DenOfIz_ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &frame.CommandList;
    executeDesc.CommandLists.NumElements = 1;
    executeDesc.Signal                   = frame.Fence;
    if ( waitSemaphores )
    {
        executeDesc.WaitSemaphores = *waitSemaphores;
    }
    DenOfIz_CommandQueue_ExecuteCommandLists( m_queue, &executeDesc );
    DenOfIz_Fence_Wait( frame.Fence );
    frame.Submitted = false;

    void *mapped = nullptr;
    DenOfIz_Buffer_MapMemory( m_captureReadback, &mapped );
    for ( uint32_t y = 0; y < pixelHeight; ++y )
    {
        const uint8_t *src = static_cast<const uint8_t *>( mapped ) + static_cast<size_t>( y ) * paddedWidth * 4;
        uint8_t       *dst = outPixels->Elements + static_cast<size_t>( y ) * pixelWidth * 4;
        for ( uint32_t x = 0; x < pixelWidth; ++x, src += 4, dst += 4 )
        {
            const uint8_t a        = src[ 3 ];
            const uint8_t r        = bgra ? src[ 2 ] : src[ 0 ];
            const uint8_t g        = src[ 1 ];
            const uint8_t b        = bgra ? src[ 0 ] : src[ 2 ];
            const auto    straight = [ a ]( const uint8_t c ) { return a == 0 ? uint8_t{ 0 } : static_cast<uint8_t>( std::min( 255, ( c * 255 + a / 2 ) / a ) ); };
            dst[ 0 ]               = a == 255 ? r : straight( r );
            dst[ 1 ]               = a == 255 ? g : straight( g );
            dst[ 2 ]               = a == 255 ? b : straight( b );
            dst[ 3 ]               = a;
        }
    }
    DenOfIz_Buffer_UnmapMemory( m_captureReadback );
    outPixels->NumElements = bytes;
    return true;
}
