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

#include "DenOfIzExamples/VGExample.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/IAssetImporter.h"
#include "DenOfIzGraphics/Assets/Import/VGImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h"
#include "DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzExamples/InteropMathConverter.h"

#include <cmath>
#include <vector>

using namespace DenOfIz;
using namespace DirectX;

void VGExample::Init( )
{
    UpdateProjectionMatrix( );

    FrameDebugRendererDesc debugRendererDesc{ };
    debugRendererDesc.GraphicsApi   = m_graphicsApi;
    debugRendererDesc.LogicalDevice = m_logicalDevice;
    debugRendererDesc.ScreenWidth   = m_windowDesc.Width;
    debugRendererDesc.ScreenHeight  = m_windowDesc.Height;
    debugRendererDesc.TextColor     = { 0.8f, 1.0f, 0.8f, 1.0f };
    debugRendererDesc.Enabled       = true;
    m_debugRenderer                 = std::make_unique<FrameDebugRenderer>( debugRendererDesc );

    InitializeRenderers( );
    LoadSvgTextures( );
    CreateStarTexture( );
    RegisterTextures( );

    m_animationTime = 0.0f;
    m_rotationAngle = 0.0f;
    m_scaleAnimTime = 0.0f;
    m_colorAnimTime = 0.0f;

    AddQuads( );
}

void VGExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void VGExample::HandleEvent( Event &event )
{
    IExample::HandleEvent( event );
}

void VGExample::Update( )
{
    m_debugRenderer->UpdateStats( m_stepTimer.GetDeltaTime( ) );

    const float deltaTime = m_stepTimer.GetDeltaTime( );
    m_animationTime += deltaTime;
    m_rotationAngle += deltaTime * 0.5f;
    m_scaleAnimTime += deltaTime * 2.0f;
    m_colorAnimTime += deltaTime * 1.5f;

    // Update star rotation
    QuadDataDesc starDesc;
    starDesc.QuadId         = 3;
    starDesc.Position       = { 850.0f, 150.0f };
    starDesc.Size           = { 256.0f, 256.0f };
    starDesc.TextureIndex   = m_starTextureIndex;
    starDesc.Color          = { 1.0f, 1.0f, 1.0f, 1.0f };
    starDesc.Scale          = { 1.0f, 1.0f };
    starDesc.Rotation       = m_rotationAngle;
    starDesc.RotationCenter = { 128.0f, 128.0f };

    for ( int i = 0; i < 3; ++i )
    {
        m_quadRenderer->UpdateQuad( i, starDesc );
    }

    RenderAndPresentFrame( );
}

void VGExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingDesc            renderingDesc{ };
    RenderingAttachmentDesc &renderingAttachmentDesc = renderingDesc.RTAttachments.EmplaceElement( );
    renderingAttachmentDesc.Resource                 = renderTarget;
    renderingAttachmentDesc.SetClearColor( 0.31, 0.3, 0.33, 1.0 );
    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    m_textRenderer->BeginBatch( );
    m_textRenderer->EndBatch( commandList );

    m_quadRenderer->Render( frameIndex, commandList );

    m_debugRenderer->Render( commandList );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void VGExample::UpdateProjectionMatrix( )
{
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( 0.0f, static_cast<float>( m_windowDesc.Width ), static_cast<float>( m_windowDesc.Height ), 0.0f, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projectionMatrix, projection );
}

void VGExample::InitializeRenderers( )
{
    TextRendererDesc textRendererDesc;
    textRendererDesc.LogicalDevice = m_logicalDevice;
    m_textRenderer                 = std::make_unique<TextRenderer>( textRendererDesc );

    Float_4x4 projMatrix;
    std::memcpy( &projMatrix, &m_projectionMatrix, sizeof( Float_4x4 ) );
    m_textRenderer->SetProjectionMatrix( projMatrix );

    QuadRendererDesc quadRendererDesc;
    quadRendererDesc.LogicalDevice = m_logicalDevice;
    m_quadRenderer                 = std::make_unique<QuadRenderer>( quadRendererDesc );
    m_quadRenderer->SetCanvas( m_windowDesc.Width, m_windowDesc.Height );
}

void VGExample::LoadSvgTextures( )
{
    ImportSvgIfNeeded( m_folderSVG, m_folderAsset );
    ImportSvgIfNeeded( m_milkTeaSVG, m_milkTeaAsset );

    if ( !FileIO::FileExists( m_folderAsset ) || !FileIO::FileExists( m_milkTeaAsset ) )
    {
        spdlog::critical( "SVG textures not found. Please run VGImporter to generate them." );
    }

    BatchResourceCopy batchCopy( m_logicalDevice );
    batchCopy.Begin( );

    BinaryReader           folderReader( m_folderAsset );
    TextureAssetReaderDesc readerDesc{ };
    readerDesc.Reader = &folderReader;
    TextureAssetReader textureAssetReader( readerDesc );

    CreateAssetTextureDesc createDesc{ };
    createDesc.Reader    = &textureAssetReader;
    createDesc.DebugName = "FolderSVG_Texture";
    m_folderTexture      = std::unique_ptr<ITextureResource>( batchCopy.CreateAndLoadAssetTexture( createDesc ) );

    BinaryReader milkTeaReader( m_milkTeaAsset );
    readerDesc.Reader    = &milkTeaReader;
    textureAssetReader   = TextureAssetReader( readerDesc );
    createDesc.Reader    = &textureAssetReader;
    createDesc.DebugName = "MilkTeaSVG_Texture";
    m_milkTeaTexture     = std::unique_ptr<ITextureResource>( batchCopy.CreateAndLoadAssetTexture( createDesc ) );

    batchCopy.Submit( );
}

void VGExample::CreateStarTexture( )
{
    constexpr uint32_t width  = 256;
    constexpr uint32_t height = 256;

    ThorVGCanvasDesc canvasDesc{ };
    canvasDesc.Width  = width;
    canvasDesc.Height = height;
    ThorVGCanvas canvas( canvasDesc );

    ThorVGShape star;

    constexpr float cx     = width / 2.0f;
    constexpr float cy     = height / 2.0f;
    constexpr float radius = 100.0f;
    constexpr float inner  = radius * 0.4f;

    star.MoveTo( cx, cy - radius );
    for ( int i = 0; i < 10; ++i )
    {
        const float angle = ( i * 36.0f - 90.0f ) * 3.14159f / 180.0f;
        const float r     = i % 2 == 0 ? radius : inner;
        const float x     = cx + r * cos( angle );
        const float y     = cy + r * sin( angle );
        star.LineTo( x, y );
    }

    star.Close( );

    ThorVGLinearGradient gradient;
    gradient.Linear( cx - radius, cy - radius, cx + radius, cy + radius );

    InteropArray<ThorVGColorStop> colorStops;
    colorStops.Resize( 3 );
    colorStops.SetElement( 0, { 0.0f, 255, 215, 0, 255 } );
    colorStops.SetElement( 1, { 0.5f, 255, 255, 100, 255 } );
    colorStops.SetElement( 2, { 1.0f, 255, 140, 0, 255 } );
    gradient.ColorStops( colorStops );

    star.Fill( &gradient );
    star.Stroke( 255, 255, 255, 255 );
    star.Stroke( 2.0f );

    canvas.Push( &star );
    canvas.Draw( );
    canvas.Sync( );

    const auto &data = canvas.GetData( );

    TextureDesc textureDesc{ };
    textureDesc.Width      = width;
    textureDesc.Height     = height;
    textureDesc.Format     = Format::R8G8B8A8Unorm;
    textureDesc.Descriptor = ResourceDescriptor::Texture;
    textureDesc.Usages     = ResourceUsage::ShaderResource;
    textureDesc.DebugName  = InteropString( "Star_Texture" );

    m_starTexture = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );

    std::vector<uint8_t> pixelData;
    pixelData.reserve( width * height * 4 );

    for ( uint32_t i = 0; i < data.NumElements( ); ++i )
    {
        const uint32_t pixel = data.GetElement( i );
        const uint8_t  a     = pixel >> 24 & 0xFF;
        const uint8_t  r     = pixel >> 16 & 0xFF;
        const uint8_t  g     = pixel >> 8 & 0xFF;
        const uint8_t  b     = pixel & 0xFF;

        pixelData.push_back( r );
        pixelData.push_back( g );
        pixelData.push_back( b );
        pixelData.push_back( a );
    }

    BatchResourceCopy batchCopy( m_logicalDevice );
    batchCopy.Begin( );

    CopyDataToTextureDesc copyDesc{ };
    copyDesc.Data.MemCpy( pixelData.data( ), pixelData.size( ) );
    copyDesc.DstTexture = m_starTexture.get( );
    copyDesc.MipLevel   = 0;
    batchCopy.CopyDataToTexture( copyDesc );

    batchCopy.Submit( );
}

void VGExample::RegisterTextures( )
{
    if( !m_folderTexture || !m_milkTeaTexture || !m_starTexture )
    {
        return;
    }

    m_folderTextureIndex  = m_quadRenderer->RegisterTexture( m_folderTexture.get( ) );
    m_milkTeaTextureIndex = m_quadRenderer->RegisterTexture( m_milkTeaTexture.get( ) );
    m_starTextureIndex    = m_quadRenderer->RegisterTexture( m_starTexture.get( ) );
}

void VGExample::AddQuads( ) const
{
    QuadDataDesc rectDesc;
    rectDesc.QuadId       = 0;
    rectDesc.Position     = { 50.0f, 50.0f };
    rectDesc.Size         = { 80.0f, 60.0f };
    rectDesc.TextureIndex = 0; // No texture (null texture)
    rectDesc.Color        = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red color
    m_quadRenderer->AddQuad( rectDesc );

    QuadDataDesc folderDesc;
    folderDesc.QuadId         = 1;
    folderDesc.Position       = { 50.0f, 350.0f };
    folderDesc.Size           = { 128.0f, 128.0f };
    folderDesc.Scale          = { 1.0f, 1.0f };
    folderDesc.TextureIndex   = m_folderTextureIndex;
    folderDesc.Color          = { 1.0f, 1.0f, 1.0f, 1.0f }; // White (no tint)
    folderDesc.Rotation       = 0.0f;
    folderDesc.RotationCenter = { 64.0f, 64.0f };
    m_quadRenderer->AddQuad( folderDesc );

    QuadDataDesc milkTeaDesc;
    milkTeaDesc.QuadId       = 2;
    milkTeaDesc.Position     = { 450.0f, 350.0f };
    milkTeaDesc.Size         = { 1024.0f, 1024.0f };
    milkTeaDesc.TextureIndex = m_milkTeaTextureIndex;
    milkTeaDesc.Color        = { 1.0f, 1.0f, 1.0f, 1.0f }; // White (no tint)
    milkTeaDesc.Scale        = { 0.3f, 0.3f };
    m_quadRenderer->AddQuad( milkTeaDesc );

    QuadDataDesc starDesc;
    starDesc.QuadId         = 3;
    starDesc.Position       = { 850.0f, 150.0f };
    starDesc.Size           = { 256.0f, 256.0f };
    starDesc.TextureIndex   = m_starTextureIndex;
    starDesc.Color          = { 1.0f, 1.0f, 1.0f, 1.0f }; // White (no tint)
    starDesc.Scale          = { 1.0f, 1.0f };
    starDesc.Rotation       = 0.0f;
    starDesc.RotationCenter = { 128.0f, 128.0f };
    m_quadRenderer->AddQuad( starDesc );
}

void VGExample::ImportSvgIfNeeded( const InteropString &svgPath, const InteropString &targetPath ) const
{
    if ( !FileIO::FileExists( targetPath ) && FileIO::FileExists( svgPath ) )
    {
        spdlog::warn( "SVG texture is missing, running import for: {}", svgPath.Get( ) );

        ImportJobDesc importJobDesc;
        importJobDesc.SourceFilePath  = svgPath;
        importJobDesc.TargetDirectory = "Assets/Textures/";

        VGImportDesc desc{ };
        desc.RenderWidth   = 1024;
        desc.RenderHeight  = 1024;
        desc.Scale         = 1.0f;
        desc.Padding       = 8;
        desc.Premultiply   = true;
        desc.OutputFormat  = Format::R8G8B8A8Unorm;
        importJobDesc.Desc = &desc;

        VGImporter     importer( VGImporterDesc{} );
        ImporterResult result = importer.Import( importJobDesc );
        if ( result.ResultCode != ImporterResultCode::Success )
        {
            spdlog::critical( "SVG import failed: {}", result.ErrorMessage.Get( ) );
        }
        else
        {
            for ( size_t i = 0; i < result.CreatedAssets.NumElements( ); ++i )
            {
                AssetUri uri = result.CreatedAssets.GetElement( i );
                spdlog::info( "Created SVG texture asset: {}", uri.Path.Get( ) );
            }
        }
    }
}
