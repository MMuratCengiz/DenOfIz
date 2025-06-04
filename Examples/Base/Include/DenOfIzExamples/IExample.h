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

#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "WorldData.h"

#include "DenOfIzGraphics/Renderer/Sync/FrameSync.h"
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"

#include "DenOfIzGraphics/Input/Event.h"

namespace DenOfIz
{
    struct ExampleWindowDesc
    {
        const char *Title     = "DenOfIzExample";
        uint32_t    Width     = 1280;
        uint32_t    Height    = 720;
        bool        Resizable = true;
    };
    class IExample
    {
    protected:
        static constexpr uint32_t NumSwapChainBuffers = 3;

        GraphicsWindowHandle          *m_windowHandle  = nullptr;
        GraphicsApi                   *m_graphicsApi   = nullptr;
        ILogicalDevice                *m_logicalDevice = nullptr;
        std::unique_ptr<ISwapChain>    m_swapChain     = nullptr;
        std::unique_ptr<ICommandQueue> m_graphicsQueue = nullptr;
        ExampleWindowDesc              m_windowDesc;
        std::unique_ptr<Camera>        m_camera;
        WorldData                      m_worldData{ };
        ResourceTracking               m_resourceTracking{ };
        std::unique_ptr<FrameSync>     m_frameSync;
        bool                           m_isRunning = true;

    public:
        virtual ~IExample( ) = default;

        void Init( GraphicsWindowHandle *window, GraphicsApi *graphicsApi, ILogicalDevice *device )
        {
            m_windowHandle  = window;
            m_graphicsApi   = graphicsApi;
            m_logicalDevice = device;
            m_windowDesc    = WindowDesc( );

            CommandQueueDesc commandQueueDesc{ };
            commandQueueDesc.QueueType                        = QueueType::Graphics;
            commandQueueDesc.Flags.RequirePresentationSupport = true;
            m_graphicsQueue                                   = std::unique_ptr<ICommandQueue>( m_logicalDevice->CreateCommandQueue( commandQueueDesc ) );

            CreateSwapChain( );
            m_camera           = std::make_unique<Camera>( static_cast<float>( m_windowDesc.Width ) / m_windowDesc.Height );
            m_worldData.Camera = m_camera.get( );

            FrameSyncDesc frameSyncDesc{ };
            frameSyncDesc.SwapChain    = m_swapChain.get( );
            frameSyncDesc.Device       = m_logicalDevice;
            frameSyncDesc.NumFrames    = NumSwapChainBuffers;
            frameSyncDesc.CommandQueue = m_graphicsQueue.get( );
            m_frameSync                = std::make_unique<FrameSync>( frameSyncDesc );
            Init( );
        }

        void CreateSwapChain( )
        {
            SwapChainDesc swapChainDesc{ };
            swapChainDesc.Width        = m_windowDesc.Width;
            swapChainDesc.Height       = m_windowDesc.Height;
            swapChainDesc.WindowHandle = m_windowHandle;
            swapChainDesc.CommandQueue = m_graphicsQueue.get( );
            swapChainDesc.ImageUsages  = ResourceUsage::CopyDst;
            swapChainDesc.NumBuffers   = NumSwapChainBuffers;
            m_swapChain                = std::unique_ptr<ISwapChain>( m_logicalDevice->CreateSwapChain( swapChainDesc ) );

            for ( uint32_t i = 0; i < swapChainDesc.NumBuffers; i++ )
            {
                m_resourceTracking.TrackTexture( m_swapChain->GetRenderTarget( i ), ResourceUsage::Common );
            }
        }
        void RenderAndPresentFrame( )
        {
            const uint64_t frameIndex = m_frameSync->NextFrame( );
            Render( frameIndex, m_frameSync->GetCommandList( frameIndex ) );
            m_frameSync->ExecuteCommandList( frameIndex );
            Present( frameIndex );
        }
        virtual void Render( uint32_t frameIndex, ICommandList *commandList )
        {
        }
        void Present( const uint64_t frameIndex )
        {
            switch ( m_frameSync->Present( frameIndex ) )
            {
            case PresentResult::Success:
                break;
            case PresentResult::Suboptimal:
                DLOG( INFO ) << "Swap chain is suboptimal, recreating...";
                if ( m_windowHandle )
                {
                    const GraphicsWindowSurface surface = m_windowHandle->GetSurface( );
                    m_windowDesc.Width                  = surface.Width;
                    m_windowDesc.Height                 = surface.Height;
                }
                if ( m_windowDesc.Width > 0 && m_windowDesc.Height > 0 )
                {
                    HandleResize( m_windowDesc.Width, m_windowDesc.Height );
                }
                break;

            case PresentResult::Timeout:
                DLOG( WARNING ) << "Present timed out, continuing...";
                break;

            case PresentResult::DeviceLost:
                DLOG( ERROR ) << "Device lost during presentation, recreating swap chain...";
                m_logicalDevice->WaitIdle( );
                CreateSwapChain( );
                break;
            }
        }
        virtual void Init( ) = 0;
        virtual void ModifyApiPreferences( APIPreference &defaultApiPreference )
        {
        }
        virtual void HandleEvent( Event &event )
        {
            if ( event.Type == EventType::KeyDown )
            {
                switch ( event.Key.Keycode )
                {
                case KeyCode::Escape:
                    m_isRunning = false;
                    break;
                default:
                    break;
                }
            }
            else if ( event.Type == EventType::WindowEvent )
            {
                if ( event.Window.Event == WindowEventType::Resized || event.Window.Event == WindowEventType::SizeChanged )
                {
                    const uint32_t newWidth  = event.Window.Data1;
                    const uint32_t newHeight = event.Window.Data2;
                    if ( newWidth > 0 && newHeight > 0 )
                    {
                        DLOG( INFO ) << "Window resized to " << newWidth << "x" << newHeight;
                        HandleResize( newWidth, newHeight );
                    }
                }
            }
        }

        void HandleResize( const uint32_t width, const uint32_t height )
        {
            m_frameSync->WaitIdle( );
            m_graphicsQueue->WaitIdle( );
            m_windowDesc.Width  = width;
            m_windowDesc.Height = height;
            m_swapChain->Resize( width, height );
            if ( m_camera )
            {
                m_camera->SetAspectRatio( static_cast<float>( width ) / height );
            }
            for ( uint32_t i = 0; i < NumSwapChainBuffers; i++ )
            {
                m_resourceTracking.TrackTexture( m_swapChain->GetRenderTarget( i ), ResourceUsage::Common );
            }
            OnResize( width, height );
        }

        virtual void OnResize( uint32_t width, uint32_t height )
        {
        }
        virtual void Update( ) = 0;
        virtual void Quit( )
        {
            m_graphicsQueue->WaitIdle( );
            m_logicalDevice->WaitIdle( );
            m_logicalDevice = nullptr;
        }
        virtual ExampleWindowDesc WindowDesc( )
        {
            constexpr struct ExampleWindowDesc windowDesc;
            return windowDesc;
        }
        [[nodiscard]] bool IsRunning( ) const
        {
            return m_isRunning;
        }
    };
} // namespace DenOfIz
