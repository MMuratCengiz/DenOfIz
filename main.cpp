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
#include <DenOfIzGraphics/Utilities/Engine.h>
#include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>

#include <DenOfIzGraphics/Renderer/ComputeTest.h>
#include <DenOfIzGraphics/Renderer/SimpleRenderer.h>
#include <filesystem>

int main( )
{
    DenOfIz::Engine::Init( );

#if defined WIN32 && defined DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
#endif
    if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
    {
        exit( 1 );
    }

    uint32_t windowFlags = SDL_WINDOW_SHOWN;
#ifdef BUILD_VK
    windowFlags |= SDL_WINDOW_VULKAN;
#endif

#if __APPLE__
    windowFlags |= SDL_WINDOW_METAL;
#endif

    const auto window = SDL_CreateWindow( "Hello C++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, windowFlags );

    const auto windowHandle = std::make_unique<DenOfIz::GraphicsWindowHandle>( );
    windowHandle->Create( window );
    DenOfIz::APIPreference apiPreferences;
    apiPreferences.Windows   = DenOfIz::APIPreferenceWindows::DirectX12;
    apiPreferences.Linux     = DenOfIz::APIPreferenceLinux::Vulkan;
    apiPreferences.OSX       = DenOfIz::APIPreferenceOSX::Metal;
    const auto gApi          = std::make_unique<DenOfIz::GraphicsApi>( apiPreferences );
    const auto logicalDevice = gApi->CreateAndLoadOptimalLogicalDevice( );

    {
//        DenOfIz::ComputeTest computeTest( gApi.get( ), logicalDevice.get( ) );
//        if (computeTest.Run() == 0) {
//            return 0;
//        }
    }
    { // SimpleRenderer scope
        auto renderer = DenOfIz::SimpleRenderer( gApi.get( ), logicalDevice.get( ) );
        renderer.Init( windowHandle.get( ) );
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
            }

            renderer.Render( );
        }
        renderer.Quit( );
    }

    SDL_DestroyWindow( window );
    SDL_Quit( );

    return 0;
}
