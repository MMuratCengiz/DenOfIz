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
#include <unordered_map>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/FileSystem/PathResolver.h"
#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Utilities/Common_Asserts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Embedded/EmbeddedFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/EmbeddedTextRendererShaders.h"
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <DirectXMath.h>

using namespace DenOfIz;
using namespace DirectX;

TextRenderer::TextRenderer( const TextRendererDesc &desc ) : m_desc( desc )
{
    if ( !desc.LogicalDevice )
    {
        LOG( FATAL ) << "TextRendererDesc::LogicalDevice must not be null";
        return;
    }

    m_logicalDevice = desc.LogicalDevice;

    // ReSharper disable once CppTooWideScope
    constexpr bool debugShaders = false;
    if ( debugShaders )
    {
        // ReSharper disable once CppDFAUnreachableCode
        BinaryReader      binaryReader{ EmbeddedTextRendererShaders::ShaderAssetBytes };
        ShaderAssetReader assetReader{ { &binaryReader } };
        m_fontShaderProgram = std::make_unique<ShaderProgram>( assetReader.Read( ) );
    }
    // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
    else
    {
        ShaderProgramDesc programDesc{ };
        ShaderStageDesc  &vsDesc = programDesc.ShaderStages.EmplaceElement( );
        vsDesc.Stage             = ShaderStage::Vertex;
        vsDesc.EntryPoint        = "main";
        vsDesc.Data              = EmbeddedTextRendererShaders::GetFontVertexShaderBytes( );

        ShaderStageDesc &psDesc = programDesc.ShaderStages.EmplaceElement( );
        psDesc.Stage            = ShaderStage::Pixel;
        psDesc.EntryPoint       = "main";
        psDesc.Data             = EmbeddedTextRendererShaders::GetFontPixelShaderBytes( );
        m_fontShaderProgram     = std::make_unique<ShaderProgram>( programDesc );
    }

    ShaderReflectDesc reflectDesc = m_fontShaderProgram->Reflect( );

    m_rootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflectDesc.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.ShaderProgram     = m_fontShaderProgram.get( );
    pipelineDesc.RootSignature     = m_rootSignature.get( );
    pipelineDesc.InputLayout       = m_inputLayout.get( );
    pipelineDesc.Graphics.FillMode = FillMode::Solid;

    RenderTargetDesc &renderTarget          = pipelineDesc.Graphics.RenderTargets.EmplaceElement( );
    renderTarget.Blend.Enable               = true;
    renderTarget.Blend.SrcBlend             = Blend::SrcAlpha;
    renderTarget.Blend.DstBlend             = Blend::InvSrcAlpha;
    renderTarget.Blend.BlendOp              = BlendOp::Add;
    renderTarget.Blend.SrcBlendAlpha        = Blend::One;
    renderTarget.Blend.DstBlendAlpha        = Blend::Zero;
    renderTarget.Blend.BlendOpAlpha         = BlendOp::Add;
    renderTarget.Format                     = Format::B8G8R8A8Unorm;
    pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology::Triangle;

    m_fontPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
    m_fonts.reserve( 64 );
    if ( m_desc.Font == nullptr )
    {
        static FontLibrary defaultFontLibrary;
        static auto        defaultFont = defaultFontLibrary.LoadFont( { EmbeddedFonts::GetInterVar( ) } );
        AddFont( defaultFont, 0 );
    }
    else
    {
        AddFont( m_desc.Font, 0 );
    }

    SetAntiAliasingMode( m_desc.AntiAliasingMode );
    if ( m_desc.Width == 0 || m_desc.Height == 0 )
    {
        LOG( WARNING ) << "Invalid viewport size, call TextRenderer::SetProjection or TextRenderer::SetViewport before rendering";
    }
    else
    {
        SetViewport( Viewport{ 0, 0, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) } );
    }
}

uint16_t TextRenderer::AddFont( Font *font, uint16_t fontId )
{
    DZ_NOT_NULL( font );
    if ( fontId == 0 && !m_fonts.empty( ) )
    {
        fontId = m_fonts.size( );
    }
    if ( m_fonts.size( ) <= fontId )
    {
        const size_t oldSize = m_fonts.size( );
        const size_t newSize = std::max<size_t>( fontId + 1, m_fonts.size( ) * 2 );
        m_fonts.resize( newSize );
        m_textBatches.resize( newSize );
        for ( size_t i = oldSize; i < newSize; ++i )
        {
            if ( !m_fonts[ i ] )
            {
                continue;
            }
            TextBatchDesc batchDesc;
            batchDesc.Font                  = m_fonts[ i ];
            batchDesc.LogicalDevice         = m_logicalDevice;
            batchDesc.RendererRootSignature = m_rootSignature.get( );
            m_textBatches[ i ]              = std::make_unique<TextBatch>( batchDesc );
            m_textBatches[ i ]->SetProjectionMatrix( InteropMathConverter::Float_4x4FromXMFLOAT4X4( m_projectionMatrix ) );
        }
        if ( std::ranges::find( m_validFonts, fontId ) == m_validFonts.end( ) )
        {
            m_validFonts.push_back( fontId );
        }
    }
    m_fonts[ fontId ] = font;
    if ( !m_textBatches[ fontId ] )
    {
        TextBatchDesc textBatchDesc{ };
        textBatchDesc.Font                  = font;
        textBatchDesc.LogicalDevice         = m_logicalDevice;
        textBatchDesc.RendererRootSignature = m_rootSignature.get( );
        m_textBatches[ fontId ]             = std::make_unique<TextBatch>( textBatchDesc );
        m_textBatches[ fontId ]->SetProjectionMatrix( InteropMathConverter::Float_4x4FromXMFLOAT4X4( m_projectionMatrix ) );
    }
    return fontId;
}

// Note keep DenOfIz::Font here to disambiguate in Linux
DenOfIz::Font *TextRenderer::GetFont( const uint16_t fontId ) const
{
    if ( m_fonts.size( ) <= fontId )
    {
        LOG( ERROR ) << "Font ID " << fontId << " does not exist";
        return nullptr;
    }
    Font *font = m_fonts[ fontId ];
    if ( font == nullptr )
    {
        LOG( ERROR ) << "Font ID " << fontId << " does not exist";
    }
    return font;
}

void TextRenderer::RemoveFont( const uint16_t fontId )
{
    if ( m_fonts.size( ) <= fontId )
    {
        return;
    }
    m_fonts[ fontId ] = nullptr;
}

void TextRenderer::SetAntiAliasingMode( const AntiAliasingMode antiAliasingMode )
{
    m_antiAliasingMode = antiAliasingMode;
}

void TextRenderer::SetProjectionMatrix( const Float_4x4 &projectionMatrix )
{
    m_projectionMatrix = InteropMathConverter::Float_4x4ToXMFLOAT4X4( projectionMatrix );
    // Todo the below shouldn't be necessary but we need to refactor TextRenderer to have separate register spaces for batch and renderer bindings
    for ( const uint16_t fontId : m_validFonts )
    {
        m_textBatches[ fontId ]->SetProjectionMatrix( projectionMatrix );
    }
}

void TextRenderer::SetViewport( const Viewport &viewport )
{
    if ( viewport.Width == 0 || viewport.Height == 0 )
    {
        LOG( WARNING ) << "Viewport::Width or Viewport::Height is zero, cannot set projection matrix";
        return;
    }
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( viewport.X, viewport.Width, viewport.Height, viewport.Y, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projectionMatrix, projection );
    // Todo Same as above
    for ( const uint16_t fontId : m_validFonts )
    {
        m_textBatches[ fontId ]->SetProjectionMatrix( InteropMathConverter::Float_4x4FromXMFLOAT4X4( m_projectionMatrix ) );
    }
}

void TextRenderer::BeginBatch( ) const
{
    for ( const uint16_t fontId : m_validFonts )
    {
        m_textBatches[ fontId ]->BeginBatch( );
    }
}

void TextRenderer::AddText( const TextRenderDesc &params ) const
{
    const Font *font = GetFont( params.FontId );
    if ( !font )
    {
        LOG( WARNING ) << "No font available for rendering";
        return;
    }
    m_textBatches[ params.FontId ]->AddText( params );
}

void TextRenderer::EndBatch( ICommandList *commandList ) const
{
    // Todo not all batches may have been registered. Is this worth to handle?
    commandList->BindPipeline( m_fontPipeline.get( ) );

    for ( const uint16_t fontId : m_validFonts )
    {
        m_textBatches[ fontId ]->EndBatch( commandList );
    }
}

Float_2 TextRenderer::MeasureText( const InteropString &text, const TextRenderDesc &desc ) const
{
    if ( text.NumChars( ) == 0 )
    {
        return Float_2{ 0.0f, 0.0f };
    }
    const Font *font = GetFont( desc.FontId );
    if ( !font )
    {
        LOG( ERROR ) << "Cannot measure text: no font available";
        return Float_2{ 0.0f, 0.0f };
    }

    return m_textBatches[ desc.FontId ]->MeasureText( text, desc );
}
