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

#include <chrono>
#include <cstdlib>
#include <thread>

#include "DenOfIzExamples/Main.h"
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/InputSystem.h"
#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphics/Utilities/Engine.h"

namespace
{
    class ResizeStress
    {
        DenOfIz_Window m_window;
        uint32_t       m_numFrames = 0;
        uint32_t       m_frame     = 0;
        uint32_t       m_random    = 0x2545F491u;

        uint32_t Next( )
        {
            m_random ^= m_random << 13;
            m_random ^= m_random >> 17;
            m_random ^= m_random << 5;
            return m_random;
        }

    public:
        explicit ResizeStress( DenOfIz_Window window ) : m_window( window )
        {
            if ( const char *value = std::getenv( "DENOFIZ_EXAMPLE_RESIZE_STRESS" ) )
            {
                m_numFrames = static_cast<uint32_t>( std::strtoul( value, nullptr, 10 ) );
            }
            if ( m_numFrames > 0 )
            {
                DenOfIz_Window_Restore( m_window );
            }
        }

        bool Step( )
        {
            if ( m_numFrames == 0 )
            {
                return true;
            }
            if ( ++m_frame > m_numFrames )
            {
                return false;
            }
            const uint32_t roll = Next( ) % 100;
            if ( roll < 30 )
            {
                const uint32_t burst = 1 + Next( ) % 3;
                for ( uint32_t i = 0; i < burst; ++i )
                {
                    DenOfIz_Window_SetSize( m_window, static_cast<int32_t>( 160 + Next( ) % 1400 ), static_cast<int32_t>( 120 + Next( ) % 900 ) );
                }
            }
            else if ( roll < 33 )
            {
                DenOfIz_Window_Maximize( m_window );
            }
            else if ( roll < 36 )
            {
                DenOfIz_Window_Restore( m_window );
            }
            else if ( roll < 37 )
            {
                DenOfIz_Window_SetSize( m_window, 1, 1 );
            }
            return true;
        }
    };
} // namespace

int DenOfIz::Main( IExample *example )
{
    DenOfIz_EngineDesc engineDesc{ };
    engineDesc.LogLevel = DENOFIZ_LOG_LEVEL_INFO;
    engineDesc.LogFile  = { };
    engineDesc.FS       = { };
    DenOfIz_Engine_Init( &engineDesc );

#if defined WIN32 && defined DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
#endif
    const auto exampleWindowDesc = example->WindowDesc( );

    DenOfIz_WindowDesc windowDesc{ };
    windowDesc.Width          = exampleWindowDesc.Width;
    windowDesc.Height         = exampleWindowDesc.Height;
    windowDesc.Position       = DENOFIZ_WINDOW_POSITION_CENTERED;
    windowDesc.Flags.Maximized = exampleWindowDesc.Maximized;
    windowDesc.Flags.Resizable = exampleWindowDesc.Resizable;
    DenOfIz_Window window = DenOfIz_Window_Create( &windowDesc );

    DenOfIz_Window_Sync( window );

    DenOfIz_APIPreference apiPreferences{ };
    apiPreferences.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_DIRECTX12;
    apiPreferences.Linux   = DENOFIZ_API_PREFERENCE_LINUX_VULKAN;
    apiPreferences.OSX     = DENOFIZ_API_PREFERENCE_OSX_METAL;
    example->ModifyApiPreferences( apiPreferences );
    DenOfIz_GraphicsApi       gApi = DenOfIz_GraphicsApi_Create( &apiPreferences );
    DenOfIz_LogicalDeviceDesc logicalDeviceDesc{ };
#ifndef NDEBUG
    logicalDeviceDesc.EnableValidationLayers = true;
#endif
    DenOfIz_LogicalDevice logicalDevice = DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice( gApi, &logicalDeviceDesc );
    if ( !DENOFIZ_HANDLE_IS_VALID( logicalDevice ) )
    {
        delete example;
        DenOfIz_GraphicsApi_Destroy( gApi );
        DenOfIz_Window_Destroy( window );
        DenOfIz_Engine_Shutdown( );
        return 1;
    }

    example->Init( window, gApi, logicalDevice );
    ResizeStress  resizeStress( window );
    auto          running = true;
    DenOfIz_Event event;
    while ( running )
    {
        if ( !resizeStress.Step( ) )
        {
            running = false;
        }
        while ( DenOfIz_InputSystem_PollEvent( &event ) )
        {
            if ( event.Type == DENOFIZ_EVENT_TYPE_QUIT )
            {
                running = false;
            }
            example->HandleEvent( event );
            if ( !example->IsRunning( ) )
            {
                running = false;
            }
        }
        example->Tick( );
        example->Update( );
        if ( DenOfIz_Window_IsMinimized( window ) )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 16 ) );
        }
    }

    bool deviceLost = false;
    DenOfIz_LogicalDevice_IsDeviceLost( logicalDevice, &deviceLost );
    const int exitCode = deviceLost ? 2 : 0;

    example->Quit( );
    delete example;
    DenOfIz_LogicalDevice_Destroy( logicalDevice );
    DenOfIz_GraphicsApi_ReportLiveObjects( );
    DenOfIz_GraphicsApi_Destroy( gApi );
    DenOfIz_Window_Destroy( window );
    DenOfIz_Engine_Shutdown( );
    return exitCode;
}
