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

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <DenOfIzCore/Engine.h>
#include <DenOfIzExamples/IExample.h>
#include <DenOfIzExamples/Main.h>
#include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
#include <DenOfIzGraphics/Backends/GraphicsApi.h>

int DenOfIz::Main( IExample *example )
{
    Engine::Init( );

#if defined WIN32 && defined DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
#endif
    if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
    {
        exit( 1 );
    }

    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;
    SDL_SetRelativeMouseMode( SDL_TRUE );
    SDL_SetHint( SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1" );
    SDL_SetHint( SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1" );

#ifdef BUILD_VK
    windowFlags |= SDL_WINDOW_VULKAN;
#endif

#if __APPLE__
    windowFlags |= SDL_WINDOW_METAL;
#endif

    const auto windowDesc = example->WindowDesc( );
    if ( windowDesc.Resizable )
    {
        windowFlags |= SDL_WINDOW_RESIZABLE;
    }
    const auto window = SDL_CreateWindow( windowDesc.Title.c_str( ), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowDesc.Width, windowDesc.Height, windowFlags );

    const auto windowHandle = std::make_unique<GraphicsWindowHandle>( );
    windowHandle->Create( window );
    APIPreference apiPreferences;
    apiPreferences.Windows = APIPreferenceWindows::DirectX12;
    apiPreferences.Linux   = APIPreferenceLinux::Vulkan;
    apiPreferences.OSX     = APIPreferenceOSX::Metal;
    example->ModifyApiPreferences( apiPreferences );
    const auto gApi          = std::make_unique<GraphicsApi>( apiPreferences );
    const auto logicalDevice = gApi->CreateAndLoadOptimalLogicalDevice( );

    GraphicsWindowHandle graphicsWindowHandle{ };
    graphicsWindowHandle.Create( window );

    example->Init( &graphicsWindowHandle, gApi.get( ), logicalDevice.get( ) );
    auto      running = true;
    SDL_Event event;
    while ( running )
    {
        while ( SDL_PollEvent( &event ) )
        {
            if ( event.type == SDL_QUIT )
            {
                running = false;
            }
            example->HandleEvent( event );
            if ( ! example->IsRunning( ) )
            {
                running = false;
            }
        }
        example->Update( );
    }

    example->Quit( );
    delete example;

    SDL_DestroyWindow( window );
    SDL_Quit( );
    return 0;
}
