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

#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"
#include "DenOfIzGraphics/Utilities/StepTimer.h"
#include "DenOfIzGraphicsInternal/Assets/Font/EmbeddedFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <deque>
#include <memory>
#include <string>
#include <vector>

// Include last due to DirectXMath
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"

#define FRAME_DEBUG_RENDERER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::FrameDebugRenderer, handle )

namespace DenOfIz
{
    struct DebugLine
    {
        std::string    Text;
        DenOfIz_Float4 Color;
    };

    class FrameDebugRenderer
    {
    public:
        DenOfIz_FrameDebugRendererDesc Desc;

        std::unique_ptr<FontLibrary> FontLibraryPtr;
        Font                        *FontPtr      = nullptr;
        DenOfIz_TextRenderer         TextRenderer = DENOFIZ_NULL_HANDLE;
        DenOfIz_Float4x4             ProjectionMatrix{ };

        DenOfIz_StepTimer  Time        = DENOFIZ_NULL_HANDLE;
        double             Fps         = 0.0;
        double             FrameTimeMs = 0.0;
        std::deque<double> FrameTimes;
        const size_t       MaxFrameTimeSamples = 120;

        std::string BackendName;
        std::string GpuName;

        std::vector<DebugLine> CustomDebugLines;
        uint32_t               FontSize;

        FrameDebugRenderer( const DenOfIz_FrameDebugRendererDesc &desc );
        ~FrameDebugRenderer( );

        void Render( DenOfIz_CommandList commandList );
        void SetViewport( const DenOfIz_Viewport &viewport );
        void SetProjectionMatrix( const DenOfIz_Float4x4 &projectionMatrix );
        void SetViewportSize( uint32_t width, uint32_t height );

        void AddDebugLine( DenOfIz_StringView text, const DenOfIz_Float4 &color );
        void ClearCustomDebugLines( );

        void SetEnabled( bool enabled );
        bool IsEnabled( ) const;
        void ToggleVisibility( );

    private:
        void UpdateStats( );
    };
} // namespace DenOfIz

using namespace DenOfIz;
using namespace DirectX;

FrameDebugRenderer::FrameDebugRenderer( const DenOfIz_FrameDebugRendererDesc &desc ) : Desc( desc )
{
    if ( Desc.LogicalDevice == DENOFIZ_NULL_HANDLE )
    {
        spdlog::warn( "FrameDebugRendererDesc.LogicalDevice cannot be null." );
        return;
    }
    if ( Desc.GraphicsApi == DENOFIZ_NULL_HANDLE )
    {
        spdlog::warn( "FrameDebugRendererDesc.GraphicsApi is null, debug info will not contain API information." );
    }
    if ( Desc.Width == 0 || Desc.Height == 0 )
    {
        spdlog::error( "FrameDebugRendererDesc.Width and FrameDebugRendererDesc.Height must be set." );
        return;
    }

    if ( Desc.FontSize == 0 )
    {
        Desc.FontSize = 16;
    }
    if ( Desc.TextColor.X == 0.0f && Desc.TextColor.Y == 0.0f && Desc.TextColor.Z == 0.0f && Desc.TextColor.W == 0.0f )
    {
        Desc.TextColor = DenOfIz_Float4{ 1.0f, 1.0f, 1.0f, 1.0f };
    }

    FrameTimes.resize( MaxFrameTimeSamples, 0.0 );
    Time = DenOfIz_StepTimer_Create( );

    if ( !DENOFIZ_HANDLE_IS_VALID( Desc.FontAsset ) )
    {
        Desc.FontAsset = EmbeddedFonts::GetJetbrainsMono( );
    }

    FontLibraryPtr = std::make_unique<FontLibrary>( );
    DenOfIz_FontDesc fontDesc{ };
    fontDesc.FontAsset = Desc.FontAsset;
    FontPtr            = FontLibraryPtr->LoadFont( fontDesc );

    if ( Desc.GraphicsApi != DENOFIZ_NULL_HANDLE )
    {
        DenOfIz_StringView activeApi;
        DenOfIz_GraphicsApi_ActiveAPI( Desc.GraphicsApi, &activeApi );
        BackendName = std::string( activeApi.Chars, activeApi.NumChars );
    }
    DenOfIz_PhysicalDevice physicalDevice;
    DenOfIz_LogicalDevice_DeviceInfo( Desc.LogicalDevice, &physicalDevice );
    const auto deviceName = physicalDevice.Name;
    GpuName               = std::string( deviceName.Chars, deviceName.NumChars );
    FontSize              = Desc.FontSize;

    DenOfIz_TextRendererDesc textRendererDesc{ };
    textRendererDesc.LogicalDevice      = Desc.LogicalDevice;
    textRendererDesc.InitialAtlasWidth  = DenOfIz_FontAsset_AtlasWidth( Desc.FontAsset );
    textRendererDesc.InitialAtlasHeight = DenOfIz_FontAsset_AtlasHeight( Desc.FontAsset );
    textRendererDesc.Width              = Desc.Width;
    textRendererDesc.Height             = Desc.Height;
    TextRenderer                        = DenOfIz_TextRenderer_Create( &textRendererDesc );
    DenOfIz_TextRenderer_AddFont( TextRenderer, DENOFIZ_TO_HANDLE( FontPtr ), 0 );
    SetViewport( DenOfIz_Viewport{ 0.0f, 0.0f, static_cast<float>( Desc.Width ), static_cast<float>( Desc.Height ) } );
}

FrameDebugRenderer::~FrameDebugRenderer( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( TextRenderer ) )
    {
        DenOfIz_TextRenderer_Destroy( TextRenderer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( Time ) )
    {
        DenOfIz_StepTimer_Destroy( Time );
    }
}

void FrameDebugRenderer::UpdateStats( )
{
    if ( !Desc.Enabled || !DENOFIZ_HANDLE_IS_VALID( TextRenderer ) )
    {
        return;
    }

    DenOfIz_StepTimer_Tick( Time );
    Fps = DenOfIz_StepTimer_GetFramesPerSecond( Time );
    FrameTimes.pop_front( );
    FrameTimes.push_back( DenOfIz_StepTimer_GetDeltaTime( Time ) * 1000.0 );

    double totalTime = 0.0;
    for ( const double time : FrameTimes )
    {
        totalTime += time;
    }
    FrameTimeMs = totalTime / FrameTimes.size( );
}

void FrameDebugRenderer::Render( DenOfIz_CommandList commandList )
{
    if ( !Desc.Enabled || !DENOFIZ_HANDLE_IS_VALID( TextRenderer ) || !DENOFIZ_HANDLE_IS_VALID( commandList ) )
    {
        return;
    }
    UpdateStats( );
    DenOfIz_TextRenderer_BeginBatch( TextRenderer );

    constexpr float margin = 10.0f;
    float           yPos   = 40.0f;
    char            buffer[ 256 ];

    std::vector<std::string>    lines;
    std::vector<DenOfIz_Float4> colors;

    snprintf( buffer, sizeof( buffer ), "Frame Time: %.2f ms", FrameTimeMs );
    lines.emplace_back( buffer );
    colors.push_back( FrameTimeMs > 16.7f ? DenOfIz_Float4{ 1.0f, 0.4f, 0.4f, 1.0f } : Desc.TextColor );

    snprintf( buffer, sizeof( buffer ), "FPS: %.1f", Fps );
    lines.emplace_back( buffer );
    colors.push_back( Desc.TextColor );

    if ( BackendName.length( ) > 0 )
    {
        const std::string backendText = std::string( "API: " ) + BackendName.c_str( );
        lines.push_back( backendText );
        colors.push_back( Desc.TextColor );
    }

    if ( !GpuName.empty( ) )
    {
        const std::string gpuText = std::string( "GPU: " ) + GpuName.c_str( );
        lines.push_back( gpuText );
        colors.push_back( Desc.TextColor );
    }

    for ( const auto &line : CustomDebugLines )
    {
        lines.emplace_back( line.Text.c_str( ) );
        colors.push_back( line.Color );
    }

    float                  maxTextWidth = 0.0f;
    DenOfIz_TextRenderDesc measureParams{ };
    measureParams.FontSize  = FontSize;
    measureParams.Direction = Desc.Direction;

    float maxHeight = 0.0f;
    for ( const auto &lineText : lines )
    {
        measureParams.Text            = DENOFIZ_STRING( lineText.c_str( ) );
        const DenOfIz_Float2 textSize = DenOfIz_TextRenderer_MeasureText( TextRenderer, measureParams.Text, &measureParams );
        maxTextWidth                  = std::max( maxTextWidth, textSize.X );
        maxHeight                     = std::max( maxHeight, textSize.Y );
    }

    const float rightMargin = static_cast<float>( Desc.Width ) - maxTextWidth - margin;
    for ( size_t i = 0; i < lines.size( ); ++i )
    {
        DenOfIz_TextRenderDesc textParams{ };
        textParams.Text      = DENOFIZ_STRING( lines[ i ].c_str( ) );
        textParams.X         = rightMargin;
        textParams.Y         = yPos;
        textParams.Color     = colors[ i ];
        textParams.FontSize  = FontSize;
        textParams.Direction = Desc.Direction;
        DenOfIz_TextRenderer_AddText( TextRenderer, &textParams );
        yPos += maxHeight * 1.2f;
    }

    DenOfIz_TextRenderer_EndBatch( TextRenderer, commandList );
}

void FrameDebugRenderer::SetViewport( const DenOfIz_Viewport &viewport )
{
    XMFLOAT4X4     projection4x4{ };
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( viewport.X, viewport.Width, viewport.Height, viewport.Y, 0.0f, 1.0f );
    XMStoreFloat4x4( &projection4x4, projection );
    ProjectionMatrix = InteropMathConverter::Float_4x4FromXMFLOAT4X4( projection4x4 );
    DenOfIz_TextRenderer_SetViewport( TextRenderer, &viewport );
}

void FrameDebugRenderer::SetProjectionMatrix( const DenOfIz_Float4x4 &projectionMatrix )
{
    ProjectionMatrix = projectionMatrix;
    if ( DENOFIZ_HANDLE_IS_VALID( TextRenderer ) )
    {
        DenOfIz_TextRenderer_SetProjectionMatrix( TextRenderer, &projectionMatrix );
    }
}

void FrameDebugRenderer::SetViewportSize( const uint32_t width, const uint32_t height )
{
    Desc.Width  = width;
    Desc.Height = height;
    SetViewport( DenOfIz_Viewport{ 0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ) } );
}

void FrameDebugRenderer::AddDebugLine( DenOfIz_StringView text, const DenOfIz_Float4 &color )
{
    CustomDebugLines.push_back( { std::string( text.Chars, text.NumChars ), color } );
}

void FrameDebugRenderer::ClearCustomDebugLines( )
{
    CustomDebugLines.clear( );
}

void FrameDebugRenderer::SetEnabled( const bool enabled )
{
    Desc.Enabled = enabled;
}

bool FrameDebugRenderer::IsEnabled( ) const
{
    return Desc.Enabled;
}

void FrameDebugRenderer::ToggleVisibility( )
{
    Desc.Enabled = !Desc.Enabled;
}

extern "C"
{
    DenOfIz_FrameDebugRenderer DenOfIz_FrameDebugRenderer_Create( const DenOfIz_FrameDebugRendererDesc *desc )
    {
        if ( desc == nullptr )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        auto *renderer = new FrameDebugRenderer( *desc );
        return DENOFIZ_TO_HANDLE( renderer );
    }

    void DenOfIz_FrameDebugRenderer_Destroy( DenOfIz_FrameDebugRenderer renderer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return;
        }
        delete FRAME_DEBUG_RENDERER_IMPL( renderer );
    }

    void DenOfIz_FrameDebugRenderer_Render( DenOfIz_FrameDebugRenderer renderer, DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return;
        }
        FRAME_DEBUG_RENDERER_IMPL( renderer )->Render( commandList );
    }

    void DenOfIz_FrameDebugRenderer_SetViewport( DenOfIz_FrameDebugRenderer renderer, const DenOfIz_Viewport *viewport )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) || viewport == nullptr )
        {
            return;
        }
        FRAME_DEBUG_RENDERER_IMPL( renderer )->SetViewport( *viewport );
    }

    void DenOfIz_FrameDebugRenderer_SetProjectionMatrix( DenOfIz_FrameDebugRenderer renderer, const DenOfIz_Float4x4 *projectionMatrix )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) || projectionMatrix == nullptr )
        {
            return;
        }
        FRAME_DEBUG_RENDERER_IMPL( renderer )->SetProjectionMatrix( *projectionMatrix );
    }

    void DenOfIz_FrameDebugRenderer_SetViewportSize( DenOfIz_FrameDebugRenderer renderer, uint32_t width, uint32_t height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return;
        }
        FRAME_DEBUG_RENDERER_IMPL( renderer )->SetViewportSize( width, height );
    }

    void DenOfIz_FrameDebugRenderer_AddDebugLine( DenOfIz_FrameDebugRenderer renderer, DenOfIz_StringView text, const DenOfIz_Float4 *color )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return;
        }
        DenOfIz_Float4 colorValue = color != nullptr ? *color : DenOfIz_Float4{ 1.0f, 1.0f, 1.0f, 1.0f };
        FRAME_DEBUG_RENDERER_IMPL( renderer )->AddDebugLine( text, colorValue );
    }

    void DenOfIz_FrameDebugRenderer_ClearCustomDebugLines( DenOfIz_FrameDebugRenderer renderer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return;
        }
        FRAME_DEBUG_RENDERER_IMPL( renderer )->ClearCustomDebugLines( );
    }

    void DenOfIz_FrameDebugRenderer_SetEnabled( DenOfIz_FrameDebugRenderer renderer, bool enabled )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return;
        }
        FRAME_DEBUG_RENDERER_IMPL( renderer )->SetEnabled( enabled );
    }

    bool DenOfIz_FrameDebugRenderer_IsEnabled( DenOfIz_FrameDebugRenderer renderer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return false;
        }
        return FRAME_DEBUG_RENDERER_IMPL( renderer )->IsEnabled( );
    }

    void DenOfIz_FrameDebugRenderer_ToggleVisibility( DenOfIz_FrameDebugRenderer renderer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( renderer ) )
        {
            return;
        }
        FRAME_DEBUG_RENDERER_IMPL( renderer )->ToggleVisibility( );
    }
}
