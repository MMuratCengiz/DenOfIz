// Include first to ensure NOMINMAX
#include <DenOfIzCore/Engine.h>

#define SDL_MAIN_HANDLED
#include <DenOfIzGraphics/Renderer/ComputeTest.h>
#include <DenOfIzGraphics/Renderer/SimpleRenderer.h>
#include <SDL2/SDL.h>
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
    apiPreferences.Windows   = DenOfIz::APIPreferenceWindows::Vulkan;
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
