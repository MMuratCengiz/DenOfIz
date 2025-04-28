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
#pragma once

#include <DenOfIzGraphics/Assets/Font/FontLibrary.h>
#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <DenOfIzGraphics/Backends/GraphicsApi.h>
#include <DenOfIzGraphics/Utilities/Time.h>
#include <chrono>
#include <deque>
#include <memory>

namespace DenOfIz
{
    struct FrameDebugRendererDesc
    {
        GraphicsApi    *GraphicsApi   = nullptr;
        ILogicalDevice *LogicalDevice = nullptr;
        uint32_t        ScreenWidth   = 0;
        uint32_t        ScreenHeight  = 0;

        // Font options - either provide FontAsset or FontPath
        FontAsset *FontAsset = nullptr;

        Float_4       TextColor   = { 1.0f, 1.0f, 1.0f, 1.0f };
        float         RefreshRate = 0.5f; // Update stats every half-second
        float         Scale       = 0.6f;
        TextDirection Direction   = TextDirection::Auto;
        bool          Enabled     = true;
    };

    class FrameDebugRenderer
    {
    private:
        FrameDebugRendererDesc           m_desc;
        std::unique_ptr<FontLibrary>     m_fontLibrary;
        std::unique_ptr<BinaryReader>    m_binaryReader;
        std::unique_ptr<FontAssetReader> m_fontAssetReader;
        std::unique_ptr<FontAsset>       m_fontAsset;
        Font                            *m_font = nullptr; // Owned by FontLibrary
        std::unique_ptr<TextRenderer>    m_textRenderer;
        XMFLOAT4X4                       m_projectionMatrix{ };

        // Performance tracking
        Time               m_time;
        double             m_fps              = 0.0;
        double             m_frameTimeMs      = 0.0;
        double             m_cpuUsagePercent  = 0.0;
        double             m_gpuUsagePercent  = 0.0;
        uint64_t           m_gpuMemoryUsageMB = 0;
        std::deque<double> m_frameTimes;
        const size_t       m_maxFrameTimeSamples = 120;

        float m_statsRefreshTimer = 0.0f;

        // Graphics backend info
        InteropString m_backendName;
        InteropString m_gpuName;

        // Custom debug information
        struct DebugLine
        {
            InteropString Text;
            Float_4       Color;
        };
        std::vector<DebugLine> m_customDebugLines;

    public:
        FrameDebugRenderer( const FrameDebugRendererDesc &desc );
        ~FrameDebugRenderer( ) = default;

        void Initialize( );
        void UpdateStats( float deltaTime );
        void Render( ICommandList *commandList );
        void SetProjectionMatrix( const XMFLOAT4X4 &projectionMatrix );
        void SetScreenSize( uint32_t width, uint32_t height );

        // Add custom debug information
        void AddDebugLine( const InteropString &text, const Float_4 &color = { 1.0f, 1.0f, 1.0f, 1.0f } );
        void ClearCustomDebugLines( );

        void SetEnabled( bool enabled )
        {
            m_desc.Enabled = enabled;
        }
        bool IsEnabled( ) const
        {
            return m_desc.Enabled;
        }

        // Toggle with keyboard
        void ToggleVisibility( )
        {
            m_desc.Enabled = !m_desc.Enabled;
        }

    private:
        void LoadFont( );
        void GatherSystemInfo( );
        void UpdatePerformanceStats( );
        void UpdateFrameTimeStats( float deltaTime );
    };
} // namespace DenOfIz
