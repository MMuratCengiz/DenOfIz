/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#define SPDLOG_NO_EXCEPTIONS
#define FMT_EXCEPTIONS 0
#include <spdlog/spdlog.h>

#include <cmath>
#include <cstdlib>
#include <memory>

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
        DenOfIz_Frame m_frame{ };
        DenOfIz_CommandListPool      m_destroyStressPool = DENOFIZ_NULL_HANDLE;

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
            m_camera           = std::make_unique<FreeCamera>( static_cast<float>( m_pixelWidth ) / static_cast<float>( m_pixelHeight ) );
            m_worldData.Camera = m_camera.get( );

            DenOfIz_FrameSyncDesc frameSyncDesc{ };
            frameSyncDesc.SwapChain        = m_swapChain;
            frameSyncDesc.Device           = m_logicalDevice;
            frameSyncDesc.NumFrames        = NumSwapChainBuffers;
            frameSyncDesc.CommandQueue     = m_graphicsQueue;
            frameSyncDesc.ResourceTracking = m_resourceTracking;
            DenOfIz_FrameSync_Create( &frameSyncDesc, &m_frameSync );
            m_stepTimer = DenOfIz_StepTimer_Create( );
            Init( );
        }

        void CreateSwapChain( )
        {
            DenOfIz_SwapChainDesc swapChainDesc{ };
            swapChainDesc.WindowHandle      = m_windowHandle;
            swapChainDesc.NumBuffers        = NumSwapChainBuffers;
            swapChainDesc.BackBufferFormat  = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
            swapChainDesc.DepthBufferFormat = DENOFIZ_FORMAT_D32_FLOAT;
            swapChainDesc.CommandQueue      = m_graphicsQueue;
            swapChainDesc.ImageUsages       = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
            swapChainDesc.PresentMode       = DENOFIZ_PRESENT_MODE_IMMEDIATE;
            swapChainDesc.EnableHDR         = false;
            swapChainDesc.ColorSpace        = DENOFIZ_COLOR_SPACE_SRGB;
            DenOfIz_LogicalDevice_CreateSwapChain( m_logicalDevice, &swapChainDesc, &m_swapChain );

            DenOfIz_SwapChain_GetDesc( m_swapChain, &swapChainDesc );
            m_pixelWidth  = swapChainDesc.Width;
            m_pixelHeight = swapChainDesc.Height;
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
            if ( !BeginFrame( frameIndex ) )
            {
                return;
            }
            Render( frameIndex, m_frame.CommandList );
            EndFrame( );
        }

        bool BeginFrame( uint32_t &outFrameIndex )
        {
            if ( !m_isRunning )
            {
                return false;
            }

            const DenOfIz_Result result = DenOfIz_FrameSync_BeginFrame( m_frameSync, &m_frame );
            if ( result == DENOFIZ_ERROR_DEVICE_LOST )
            {
                m_isRunning = false;
            }
            if ( result != DENOFIZ_SUCCESS )
            {
                return false;
            }

            outFrameIndex = m_frame.Index;
            if ( m_frame.Resized )
            {
                m_pixelWidth  = m_frame.Width;
                m_pixelHeight = m_frame.Height;
                if ( m_camera )
                {
                    m_camera->SetAspectRatio( static_cast<float>( m_pixelWidth ) / static_cast<float>( m_pixelHeight ) );
                }
                OnResize( m_pixelWidth, m_pixelHeight );
            }
            return true;
        }

        virtual void Render( uint32_t frameIndex, DenOfIz_CommandList commandList )
        {
        }

        void DestroyStress( )
        {
            static const bool enabled = std::getenv( "DENOFIZ_EXAMPLE_DESTROY_STRESS" ) != nullptr;
            if ( !enabled )
            {
                return;
            }
            if ( !DENOFIZ_HANDLE_IS_VALID( m_destroyStressPool ) )
            {
                DenOfIz_CommandListPoolDesc poolDesc{ };
                poolDesc.CommandQueue    = m_graphicsQueue;
                poolDesc.NumCommandLists = NumSwapChainBuffers;
                DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &poolDesc, &m_destroyStressPool );
            }
            DenOfIz_CommandListArray commandLists{ };
            DenOfIz_CommandListPool_GetCommandLists( m_destroyStressPool, &commandLists );
            DenOfIz_CommandList commandList = commandLists.Elements[ m_frame.Index % commandLists.NumElements ];

            DenOfIz_BufferDesc srcDesc{ };
            srcDesc.NumBytes  = 256;
            srcDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
            srcDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
            srcDesc.DebugName = DENOFIZ_STRING( "DestroyStressSrc" );

            DenOfIz_BufferDesc dstDesc{ };
            dstDesc.NumBytes  = 256;
            dstDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
            dstDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
            dstDesc.DebugName = DENOFIZ_STRING( "DestroyStressDst" );

            DenOfIz_Buffer src = DENOFIZ_NULL_HANDLE;
            DenOfIz_Buffer dst = DENOFIZ_NULL_HANDLE;
            if ( DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &srcDesc, &src ) == DENOFIZ_SUCCESS &&
                 DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &dstDesc, &dst ) == DENOFIZ_SUCCESS )
            {
                DenOfIz_CopyBufferRegionDesc copyDesc{ };
                copyDesc.SrcBuffer = src;
                copyDesc.DstBuffer = dst;
                copyDesc.NumBytes  = 256;

                DenOfIz_CommandList_Begin( commandList );
                DenOfIz_CommandList_CopyBufferRegion( commandList, &copyDesc );
                DenOfIz_CommandList_End( commandList );

                DenOfIz_ExecuteCommandListsDesc executeDesc{ };
                executeDesc.CommandLists.Elements    = &commandList;
                executeDesc.CommandLists.NumElements = 1;
                DenOfIz_CommandQueue_ExecuteCommandLists( m_graphicsQueue, &executeDesc );
            }
            DenOfIz_Buffer_Destroy( src );
            DenOfIz_Buffer_Destroy( dst );
        }

        void EndFrame( const DenOfIz_SemaphoreArray *waitSemaphores = nullptr )
        {
            DestroyStress( );
            if ( DenOfIz_FrameSync_EndFrame( m_frameSync, waitSemaphores ) == DENOFIZ_ERROR_DEVICE_LOST )
            {
                m_isRunning = false;
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
        }

        virtual void OnResize( uint32_t width, uint32_t height )
        {
        }

        void RecreateScreenSizedTexture( DenOfIz_Texture &texture, DenOfIz_TextureDesc desc ) const
        {
            if ( DENOFIZ_HANDLE_IS_VALID( texture ) )
            {
                DenOfIz_ResourceTracking_UntrackTexture( m_resourceTracking, texture );
                DenOfIz_Texture_Destroy( texture );
                texture = DENOFIZ_NULL_HANDLE;
            }
            desc.Width  = m_pixelWidth;
            desc.Height = m_pixelHeight;
            DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &desc, &texture );
            DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, texture, DENOFIZ_QUEUE_TYPE_GRAPHICS );
        }

        virtual void Update( ) = 0;
        virtual void Quit( )
        {
            DenOfIz_FrameSync_WaitIdle( m_frameSync );
            DenOfIz_CommandQueue_WaitIdle( m_graphicsQueue );
            DenOfIz_LogicalDevice_WaitIdle( m_logicalDevice );
            DenOfIz_CommandListPool_Destroy( m_destroyStressPool );
            DenOfIz_FrameSync_Destroy( m_frameSync );
            DenOfIz_SwapChain_Destroy( m_swapChain );
            DenOfIz_CommandQueue_Destroy( m_graphicsQueue );
            DenOfIz_ResourceTracking_Destroy( m_resourceTracking );
            DenOfIz_StepTimer_Destroy( m_stepTimer );

            m_frameSync        = DENOFIZ_NULL_HANDLE;
            m_swapChain        = DENOFIZ_NULL_HANDLE;
            m_graphicsQueue    = DENOFIZ_NULL_HANDLE;
            m_resourceTracking = DENOFIZ_NULL_HANDLE;
            m_stepTimer        = DENOFIZ_NULL_HANDLE;
            m_logicalDevice    = DENOFIZ_NULL_HANDLE;
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
