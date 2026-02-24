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

#define SPDLOG_NO_EXCEPTIONS
#define FMT_EXCEPTIONS 0
#include <spdlog/spdlog.h>

#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/MetalCapture.h"
#include "DenOfIzGraphics/Input/Display.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphics/Support/FrameSync.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphics/Utilities/StepTimer.h"
#include "WorldData.h"

namespace DenOfIz
{
    struct ExampleWindowDesc
    {
        const char *Title     = "DenOfIzExample";
        uint32_t    Width     = 1920;
        uint32_t    Height    = 1080;
        bool        Resizable = true;
        bool        Maximized = true;

        ExampleWindowDesc( )
        {
            Width  = DenOfIz_Display_GetPrimaryDisplay( ).Size.Width * 0.8;
            Height = DenOfIz_Display_GetPrimaryDisplay( ).Size.Height * 0.8;
        }
    };
    class IExample
    {
    protected:
        static constexpr uint32_t NumSwapChainBuffers = 3;

        DenOfIz_Window               m_window        = DENOFIZ_NULL_HANDLE;
        DenOfIz_GraphicsWindowHandle m_windowHandle  = DENOFIZ_NULL_HANDLE;
        DenOfIz_GraphicsApi          m_graphicsApi   = DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice        m_logicalDevice = DENOFIZ_NULL_HANDLE;
        DenOfIz_SwapChain            m_swapChain     = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandQueue         m_graphicsQueue = DENOFIZ_NULL_HANDLE;
        ExampleWindowDesc            m_windowDesc;
        uint32_t                     m_pixelWidth        = 0;
        uint32_t                     m_pixelHeight       = 0;
        std::unique_ptr<FreeCamera>  m_camera;
        WorldData                    m_worldData{ };
        DenOfIz_ResourceTracking     m_resourceTracking  = DENOFIZ_NULL_HANDLE;
        DenOfIz_FrameSync            m_frameSync         = DENOFIZ_NULL_HANDLE;
        bool                         m_isRunning         = true;
        DenOfIz_StepTimer            m_stepTimer         = DENOFIZ_NULL_HANDLE;
        uint32_t                     m_lastElapsedSecond = 0;

    public:
        virtual ~IExample( ) = default;

        void Init( DenOfIz_Window window, DenOfIz_GraphicsApi graphicsApi, DenOfIz_LogicalDevice device )
        {
            m_window        = window;
            m_windowHandle  = DenOfIz_Window_GetGraphicsWindowHandle( window );
            m_graphicsApi   = graphicsApi;
            m_logicalDevice = device;
            m_windowDesc    = WindowDesc( );

            DenOfIz_DisplaySize pixelSize = DenOfIz_Window_GetSizeInPixels( m_window );
            m_pixelWidth                  = pixelSize.Width;
            m_pixelHeight                 = pixelSize.Height;

            DenOfIz_CommandQueueDesc commandQueueDesc{ };
            commandQueueDesc.QueueType                        = DENOFIZ_QUEUE_TYPE_GRAPHICS;
            commandQueueDesc.Flags.RequirePresentationSupport = true;
            DenOfIz_LogicalDevice_CreateCommandQueue( m_logicalDevice, &commandQueueDesc, &m_graphicsQueue );

            DenOfIz_ResourceTracking_Create( &m_resourceTracking );
            CreateSwapChain( );
            m_camera           = std::make_unique<FreeCamera>( static_cast<float>( m_pixelWidth ) / m_pixelHeight );
            m_worldData.Camera = m_camera.get( );

            DenOfIz_FrameSyncDesc frameSyncDesc{ };
            frameSyncDesc.SwapChain    = m_swapChain;
            frameSyncDesc.Device       = m_logicalDevice;
            frameSyncDesc.NumFrames    = NumSwapChainBuffers;
            frameSyncDesc.CommandQueue = m_graphicsQueue;
            DenOfIz_FrameSync_Create( &frameSyncDesc, &m_frameSync );
            m_stepTimer = DenOfIz_StepTimer_Create( );
            Init( );
        }

        void CreateSwapChain( )
        {
            DenOfIz_SwapChainDesc swapChainDesc{ };
            swapChainDesc.WindowHandle      = m_windowHandle;
            swapChainDesc.Width             = m_pixelWidth;
            swapChainDesc.Height            = m_pixelHeight;
            swapChainDesc.NumBuffers        = NumSwapChainBuffers;
            swapChainDesc.BackBufferFormat  = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
            swapChainDesc.DepthBufferFormat = DENOFIZ_FORMAT_D32_FLOAT;
            swapChainDesc.CommandQueue      = m_graphicsQueue;
            swapChainDesc.ImageUsages       = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
            swapChainDesc.AllowTearing      = true;
            swapChainDesc.EnableHDR         = false;
            swapChainDesc.ColorSpace        = DENOFIZ_COLOR_SPACE_SRGB;
            swapChainDesc.SampleCount       = DENOFIZ_MSAA_SAMPLE_COUNT_1;
            DenOfIz_LogicalDevice_CreateSwapChain( m_logicalDevice, &swapChainDesc, &m_swapChain );

            for ( uint32_t i = 0; i < swapChainDesc.NumBuffers; i++ )
            {
                DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
                DenOfIz_SwapChain_GetRenderTarget( m_swapChain, i, &renderTarget );
                DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, renderTarget, DENOFIZ_QUEUE_TYPE_GRAPHICS );
            }
        }

        void Tick( )
        {
            DenOfIz_StepTimer_Tick( m_stepTimer );
            const uint32_t elapsedSeconds = std::floor( DenOfIz_StepTimer_GetTotalSeconds( m_stepTimer ) );
            if ( elapsedSeconds != m_lastElapsedSecond )
            {
                m_lastElapsedSecond = elapsedSeconds;
                spdlog::warn( "FPS: {}", DenOfIz_StepTimer_GetFramesPerSecond( m_stepTimer ) );
            }
        }

        virtual void RenderAndPresentFrame( )
        {
            uint32_t frameIndex = 0;
            DenOfIz_FrameSync_NextFrame( m_frameSync, &frameIndex );
            DenOfIz_CommandList commandList = DENOFIZ_NULL_HANDLE;
            DenOfIz_FrameSync_GetCommandList( m_frameSync, frameIndex, &commandList );
            Render( frameIndex, commandList );
            DenOfIz_FrameSync_ExecuteCommandList( m_frameSync, frameIndex, nullptr );
            Present( frameIndex );
        }

        virtual void Render( uint32_t frameIndex, DenOfIz_CommandList commandList )
        {
        }
        void Present( const uint32_t frameIndex )
        {
            DenOfIz_PresentResult presentResult = DENOFIZ_PRESENT_RESULT_SUCCESS;
            DenOfIz_FrameSync_Present( m_frameSync, frameIndex, &presentResult );
            switch ( presentResult )
            {
            case DENOFIZ_PRESENT_RESULT_SUCCESS:
                break;
            case DENOFIZ_PRESENT_RESULT_SUBOPTIMAL:
                spdlog::info( "Swap chain is suboptimal, recreating..." );
                if ( DENOFIZ_HANDLE_IS_VALID( m_window ) )
                {
                    DenOfIz_DisplaySize pixelSize = DenOfIz_Window_GetSizeInPixels( m_window );
                    if ( pixelSize.Width > 0 && pixelSize.Height > 0 )
                    {
                        HandleResize( pixelSize.Width, pixelSize.Height );
                    }
                }
                break;

            case DENOFIZ_PRESENT_RESULT_TIMEOUT:
                spdlog::warn( "Present timed out, continuing..." );
                break;

            case DENOFIZ_PRESENT_RESULT_DEVICE_LOST:
                spdlog::error( "Device lost during presentation, recreating swap chain..." );
                DenOfIz_LogicalDevice_WaitIdle( m_logicalDevice );
                DenOfIz_DisplaySize pixelSize = DenOfIz_Window_GetSizeInPixels( m_window );
                m_pixelWidth                  = pixelSize.Width;
                m_pixelHeight                 = pixelSize.Height;
                CreateSwapChain( );
                break;
            }
        }
        virtual void Init( ) = 0;
        virtual void ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
        {
        }
        virtual void HandleEvent( DenOfIz_Event &event )
        {
            if ( event.Type == DENOFIZ_EVENT_TYPE_KEY_DOWN )
            {
                switch ( event.Key.KeyCode )
                {
                case DENOFIZ_KEY_CODE_ESCAPE:
                    m_isRunning = false;
                    break;
                case DENOFIZ_KEY_CODE_F12:
                    if ( event.Key.Mod & DENOFIZ_KEY_MOD_GUI )
                    {
                        if ( DenOfIz_Metal_IsCapturing( m_logicalDevice ) )
                        {
                            DenOfIz_Metal_EndGpuCapture( m_logicalDevice );
                        }
                        else
                        {
                            DenOfIz_Metal_BeginGpuCapture( m_logicalDevice );
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            else if ( event.Type == DENOFIZ_EVENT_TYPE_WINDOW_EVENT )
            {
                if ( event.Window.Event == DENOFIZ_WINDOW_EVENT_TYPE_SIZE_CHANGED )
                {
                    const uint32_t newWidth  = event.Window.Data1;
                    const uint32_t newHeight = event.Window.Data2;
                    if ( newWidth > 0 && newHeight > 0 && ( newWidth != m_pixelWidth || newHeight != m_pixelHeight ) )
                    {
                        spdlog::info( "Window pixel size changed to {}x{}", newWidth, newHeight );
                        HandleResize( newWidth, newHeight );
                    }
                }
            }
        }

        void HandleResize( const uint32_t pixelWidth, const uint32_t pixelHeight )
        {
            DenOfIz_FrameSync_WaitIdle( m_frameSync );
            DenOfIz_CommandQueue_WaitIdle( m_graphicsQueue );
            m_pixelWidth  = pixelWidth;
            m_pixelHeight = pixelHeight;
            DenOfIz_SwapChain_Resize( m_swapChain, pixelWidth, pixelHeight );
            if ( m_camera )
            {
                m_camera->SetAspectRatio( static_cast<float>( pixelWidth ) / pixelHeight );
            }
            for ( uint32_t i = 0; i < NumSwapChainBuffers; i++ )
            {
                DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
                DenOfIz_SwapChain_GetRenderTarget( m_swapChain, i, &renderTarget );
                DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, renderTarget, DENOFIZ_QUEUE_TYPE_GRAPHICS );
            }
            OnResize( pixelWidth, pixelHeight );
            DenOfIz_FrameSync_WaitIdle( m_frameSync );
            DenOfIz_CommandQueue_WaitIdle( m_graphicsQueue );
            DenOfIz_LogicalDevice_WaitIdle( m_logicalDevice );
        }

        virtual void OnResize( uint32_t width, uint32_t height )
        {
        }
        virtual void Update( ) = 0;
        virtual void Quit( )
        {
            DenOfIz_CommandQueue_WaitIdle( m_graphicsQueue );
            DenOfIz_LogicalDevice_WaitIdle( m_logicalDevice );
            DenOfIz_FrameSync_Destroy( m_frameSync );
            DenOfIz_SwapChain_Destroy( m_swapChain );
            DenOfIz_CommandQueue_Destroy( m_graphicsQueue );
            DenOfIz_ResourceTracking_Destroy( m_resourceTracking );
            DenOfIz_StepTimer_Destroy( m_stepTimer );
            m_logicalDevice = DENOFIZ_NULL_HANDLE;
        }
        virtual ExampleWindowDesc WindowDesc( )
        {
            return { };
        }
        [[nodiscard]] bool IsRunning( ) const
        {
            return m_isRunning;
        }
    };
} // namespace DenOfIz
