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
#include <SDL2/SDL.h>
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Input/InputSystem.h"
#include "DenOfIzGraphics/Input/Window.h"

int DenOfIz::Main( IExample *example )
{
    Engine::Init( );

#if defined WIN32 && defined DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
#endif
    if ( SDL_Init( SDL_INIT_EVERYTHING ^ SDL_INIT_AUDIO ) != 0 )
    {
        spdlog::critical( "SDL_Init failed: {}", SDL_GetError( ) );
    }

    const auto exampleWindowDesc = example->WindowDesc( );

    WindowDesc windowDesc{ };
    windowDesc.Width    = exampleWindowDesc.Width;
    windowDesc.Height   = exampleWindowDesc.Height;
    windowDesc.Position = WindowPosition::Centered;
    Window window( windowDesc );

    window.SetResizable( exampleWindowDesc.Resizable );

    auto          graphicsWindowHandle = window.GetGraphicsWindowHandle( );
    APIPreference apiPreferences;
    apiPreferences.Windows = APIPreferenceWindows::DirectX12;
    apiPreferences.Linux   = APIPreferenceLinux::Vulkan;
    apiPreferences.OSX     = APIPreferenceOSX::Metal;
    example->ModifyApiPreferences( apiPreferences );
    const auto gApi          = std::make_unique<GraphicsApi>( apiPreferences );
    auto       logicalDevice = std::unique_ptr<ILogicalDevice>( gApi->CreateAndLoadOptimalLogicalDevice( ) );

    example->Init( graphicsWindowHandle, gApi.get( ), logicalDevice.get( ) );
    auto        running = true;
    Event       event;
    InputSystem inputSystem{ };
    while ( running )
    {
        while ( InputSystem::PollEvent( event ) )
        {
            if ( event.Type == EventType::Quit )
            {
                running = false;
            }
            example->HandleEvent( event );
            if ( !example->IsRunning( ) )
            {
                running = false;
            }
        }
        example->Update( );
    }

    example->Quit( );
    delete example;
    logicalDevice.reset( );
    GraphicsApi::ReportLiveObjects( );
    return 0;
}
