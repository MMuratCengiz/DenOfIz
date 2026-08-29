/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphicsInternal/Assets/Font/EmbeddedFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextBatch.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/EmbeddedShaders.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <DirectXMath.h>
#include <array>
#include <map>
#include <memory>

#define TEXT_RENDERER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::TextRenderer, handle )

namespace DenOfIz
{
    class TextRenderer
    {
    public:
        DenOfIz_TextRendererDesc Desc;

        DenOfIz_LogicalDevice                LogicalDevice = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     Pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                RootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  InputLayout   = DENOFIZ_NULL_HANDLE;
        DenOfIz_Sampler                      FontSampler   = DENOFIZ_NULL_HANDLE;
        DenOfIz_ShaderProgram                ShaderProgram = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> BindGroupLayouts;

        uint32_t AtlasWidth  = 1024;
        uint32_t AtlasHeight = 1024;

        DenOfIz_Float4x4 ProjectionMatrix;
        DenOfIz_Viewport Viewport;

        std::unique_ptr<FontLibrary>                   FontLibraryPtr;
        std::map<uint16_t, std::unique_ptr<TextBatch>> FontBatches;
        std::map<uint16_t, Font *>                     FontMap;
        uint16_t                                       NextFontId = 64;

        TextRenderer( const DenOfIz_TextRendererDesc &desc );
        ~TextRenderer( );

        void SetProjectionMatrix( const DenOfIz_Float4x4 &projectionMatrix );
        void SetViewport( const DenOfIz_Viewport &viewport );

        uint16_t AddFont( Font *font, uint16_t fontId );
        Font    *GetFont( uint16_t fontId );
        void     RemoveFont( uint16_t fontId );

        void BeginBatch( );
        void AddText( const DenOfIz_TextRenderDesc &textDesc );
        void EndBatch( DenOfIz_CommandList commandList );

        DenOfIz_Float2 MeasureText( DenOfIz_StringView text, const DenOfIz_TextRenderDesc &desc );

    private:
        void UpdateOrthoProjection( );
        void CreateDefaultBatch( );
    };
} // namespace DenOfIz

using namespace DenOfIz;
using namespace DirectX;

TextRenderer::TextRenderer( const DenOfIz_TextRendererDesc &desc ) : Desc( desc )
{
    LogicalDevice  = desc.LogicalDevice;
    FontLibraryPtr = std::make_unique<FontLibrary>( );

    if ( desc.InitialAtlasWidth > 0 )
    {
        AtlasWidth = desc.InitialAtlasWidth;
    }
    if ( desc.InitialAtlasHeight > 0 )
    {
        AtlasHeight = desc.InitialAtlasHeight;
    }

    Viewport.X      = 0;
    Viewport.Y      = 0;
    Viewport.Width  = static_cast<float>( desc.Width );
    Viewport.Height = static_cast<float>( desc.Height );

    ShaderProgram = EmbeddedShaders::GetTextRendererShaderProgram( );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( ShaderProgram, &reflectDesc );

    BindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( LogicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &BindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = BindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = BindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( LogicalDevice, &rootSigDesc, &RootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( LogicalDevice, &reflectDesc.InputLayout, &InputLayout );

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
    pipelineDesc.RootSignature = RootSignature;
    pipelineDesc.InputLayout   = InputLayout;
    pipelineDesc.ShaderProgram = ShaderProgram;
    pipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_GRAPHICS;

    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.DepthTest.Enable          = false;
    pipelineDesc.Graphics.DepthTest.Write           = false;

    DenOfIz_LogicalDevice_CreatePipeline( LogicalDevice, &pipelineDesc, &Pipeline );

    CreateDefaultBatch( );
    UpdateOrthoProjection( );
}

TextRenderer::~TextRenderer( )
{
    FontBatches.clear( );
    FontMap.clear( );

    if ( DENOFIZ_HANDLE_IS_VALID( Pipeline ) )
    {
        DenOfIz_Pipeline_Destroy( Pipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( RootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( RootSignature );
    }
    for ( auto &layout : BindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( InputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( InputLayout );
    }
    // ShaderProgram comes from EmbeddedShaders' static cache
    ShaderProgram = DENOFIZ_NULL_HANDLE;
}

void TextRenderer::SetProjectionMatrix( const DenOfIz_Float4x4 &projectionMatrix )
{
    ProjectionMatrix = projectionMatrix;
    for ( auto &[ id, batch ] : FontBatches )
    {
        batch->SetProjectionMatrix( ProjectionMatrix );
    }
}

void TextRenderer::SetViewport( const DenOfIz_Viewport &viewport )
{
    Viewport = viewport;
    UpdateOrthoProjection( );
}

uint16_t TextRenderer::AddFont( Font *font, uint16_t fontId )
{
    FontMap[ fontId ] = font;

    TextBatchDesc batchDesc{ };
    batchDesc.BindGroupLayout = BindGroupLayouts[ 0 ];
    batchDesc.LogicalDevice   = LogicalDevice;
    batchDesc.Font            = font;

    FontBatches[ fontId ] = std::make_unique<TextBatch>( batchDesc );
    FontBatches[ fontId ]->SetProjectionMatrix( ProjectionMatrix );

    return fontId;
}

Font *TextRenderer::GetFont( uint16_t fontId )
{
    auto it = FontMap.find( fontId );
    if ( it != FontMap.end( ) )
    {
        return it->second;
    }
    return nullptr;
}

void TextRenderer::RemoveFont( uint16_t fontId )
{
    FontBatches.erase( fontId );
    FontMap.erase( fontId );
}

void TextRenderer::BeginBatch( )
{
    for ( auto &[ id, batch ] : FontBatches )
    {
        batch->BeginBatch( );
    }
}

void TextRenderer::AddText( const DenOfIz_TextRenderDesc &textDesc )
{
    uint16_t fontId = textDesc.FontId;
    if ( fontId == 0 )
    {
        fontId = 0;
    }

    auto it = FontBatches.find( fontId );
    if ( it == FontBatches.end( ) )
    {
        spdlog::error( "TextRenderer::AddText: FontId {} not found", fontId );
        return;
    }

    DenOfIz_AddTextDesc addTextDesc{ };
    addTextDesc.FontSize         = textDesc.FontSize;
    addTextDesc.Text             = textDesc.Text;
    addTextDesc.X                = textDesc.X;
    addTextDesc.Y                = textDesc.Y;
    addTextDesc.Color            = textDesc.Color;
    addTextDesc.LetterSpacing    = textDesc.LetterSpacing;
    addTextDesc.LineHeight       = textDesc.LineHeight;
    addTextDesc.HorizontalCenter = textDesc.HorizontalCenter;
    addTextDesc.VerticalCenter   = textDesc.VerticalCenter;
    addTextDesc.Direction        = textDesc.Direction;

    it->second->AddText( addTextDesc );
}

void TextRenderer::EndBatch( DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_BindPipeline( commandList, Pipeline );
    DenOfIz_CommandList_BindViewport( commandList, Viewport.X, Viewport.Y, Viewport.Width, Viewport.Height );

    for ( auto &[ id, batch ] : FontBatches )
    {
        batch->EndBatch( commandList );
    }
}

DenOfIz_Float2 TextRenderer::MeasureText( DenOfIz_StringView text, const DenOfIz_TextRenderDesc &desc )
{
    uint16_t fontId = desc.FontId;
    if ( fontId == 0 )
    {
        fontId = 0;
    }

    auto it = FontBatches.find( fontId );
    if ( it == FontBatches.end( ) )
    {
        return DenOfIz_Float2{ 0.0f, 0.0f };
    }

    DenOfIz_AddTextDesc addTextDesc{ };
    addTextDesc.FontSize      = desc.FontSize;
    addTextDesc.LetterSpacing = desc.LetterSpacing;
    addTextDesc.LineHeight    = desc.LineHeight;

    return it->second->MeasureText( text, addTextDesc );
}

void TextRenderer::UpdateOrthoProjection( )
{
    const float width  = Viewport.Width;
    const float height = Viewport.Height;

    XMMATRIX ortho = XMMatrixOrthographicOffCenterLH( 0.0f, width, height, 0.0f, 0.0f, 1.0f );

    XMFLOAT4X4 orthoMatrix;
    XMStoreFloat4x4( &orthoMatrix, ortho );
    memcpy( &ProjectionMatrix, &orthoMatrix, sizeof( DenOfIz_Float4x4 ) );

    for ( auto &[ id, batch ] : FontBatches )
    {
        batch->SetProjectionMatrix( ProjectionMatrix );
    }
}

void TextRenderer::CreateDefaultBatch( )
{
    DenOfIz_FontAsset defaultFontAsset = EmbeddedFonts::GetJetbrainsMono( );

    DenOfIz_FontDesc fontDesc{ };
    fontDesc.FontAsset = defaultFontAsset;
    Font *defaultFont  = FontLibraryPtr->LoadFont( fontDesc );

    FontMap[ 0 ] = defaultFont;

    TextBatchDesc batchDesc{ };
    batchDesc.BindGroupLayout = BindGroupLayouts[ 0 ];
    batchDesc.LogicalDevice   = LogicalDevice;
    batchDesc.Font            = defaultFont;

    FontBatches[ 0 ] = std::make_unique<TextBatch>( batchDesc );
    FontBatches[ 0 ]->SetProjectionMatrix( ProjectionMatrix );
}

extern "C"
{
    DenOfIz_TextRenderer DenOfIz_TextRenderer_Create( const DenOfIz_TextRendererDesc *desc )
    {
        if ( desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->LogicalDevice ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        auto *renderer = new TextRenderer( *desc );
        return DENOFIZ_TO_HANDLE( renderer );
    }

    void DenOfIz_TextRenderer_Destroy( DenOfIz_TextRenderer textRenderer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            return;
        }
        delete TEXT_RENDERER_IMPL( textRenderer );
    }

    void DenOfIz_TextRenderer_SetProjectionMatrix( DenOfIz_TextRenderer textRenderer, const DenOfIz_Float4x4 *projectionMatrix )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || projectionMatrix == nullptr )
        {
            return;
        }
        TEXT_RENDERER_IMPL( textRenderer )->SetProjectionMatrix( *projectionMatrix );
    }

    void DenOfIz_TextRenderer_SetViewport( DenOfIz_TextRenderer textRenderer, const DenOfIz_Viewport *viewport )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || viewport == nullptr )
        {
            return;
        }
        TEXT_RENDERER_IMPL( textRenderer )->SetViewport( *viewport );
    }

    uint16_t DenOfIz_TextRenderer_AddFont( DenOfIz_TextRenderer textRenderer, DenOfIz_Font font, uint16_t fontId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || !DENOFIZ_HANDLE_IS_VALID( font ) )
        {
            return 0;
        }
        return TEXT_RENDERER_IMPL( textRenderer )->AddFont( FONT_IMPL( font ), fontId );
    }

    DenOfIz_Font DenOfIz_TextRenderer_GetFont( DenOfIz_TextRenderer textRenderer, uint16_t fontId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        Font *font = TEXT_RENDERER_IMPL( textRenderer )->GetFont( fontId );
        return DENOFIZ_TO_HANDLE( font );
    }

    void DenOfIz_TextRenderer_RemoveFont( DenOfIz_TextRenderer textRenderer, uint16_t fontId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            return;
        }
        TEXT_RENDERER_IMPL( textRenderer )->RemoveFont( fontId );
    }

    void DenOfIz_TextRenderer_BeginBatch( DenOfIz_TextRenderer textRenderer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) )
        {
            return;
        }
        TEXT_RENDERER_IMPL( textRenderer )->BeginBatch( );
    }

    void DenOfIz_TextRenderer_AddText( DenOfIz_TextRenderer textRenderer, const DenOfIz_TextRenderDesc *textDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || textDesc == nullptr )
        {
            return;
        }
        TEXT_RENDERER_IMPL( textRenderer )->AddText( *textDesc );
    }

    void DenOfIz_TextRenderer_EndBatch( DenOfIz_TextRenderer textRenderer, DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        TEXT_RENDERER_IMPL( textRenderer )->EndBatch( commandList );
    }

    DenOfIz_Float2 DenOfIz_TextRenderer_MeasureText( DenOfIz_TextRenderer textRenderer, DenOfIz_StringView text, const DenOfIz_TextRenderDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textRenderer ) || desc == nullptr )
        {
            return DenOfIz_Float2{ 0.0f, 0.0f };
        }
        return TEXT_RENDERER_IMPL( textRenderer )->MeasureText( text, *desc );
    }
}
