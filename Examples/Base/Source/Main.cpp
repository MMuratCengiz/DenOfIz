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

#include "DenOfIzExamples/Main.h"
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/InputSystem.h"
#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphics/Utilities/Engine.h"

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

    example->Init( window, gApi, logicalDevice );
    auto          running = true;
    DenOfIz_Event event;
    while ( running )
    {
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
    }

    example->Quit( );
    delete example;
    DenOfIz_LogicalDevice_Destroy( logicalDevice );
    DenOfIz_GraphicsApi_ReportLiveObjects( );
    DenOfIz_GraphicsApi_Destroy( gApi );
    DenOfIz_Window_Destroy( window );
    DenOfIz_Engine_Shutdown( );
    return 0;
}
