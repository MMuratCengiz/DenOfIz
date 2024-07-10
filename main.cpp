#ifndef DISABLED
// Include first to ensure NOMINMAX
#include <DenOfIzCore/Engine.h>

#define SDL_MAIN_HANDLED
#include <DenOfIzGraphics/Renderer/SimpleRenderer.h>
#include <DenOfIzGraphics/Renderer/ComputeTest.h>
#include <SDL2/SDL.h>
#include <filesystem>

int main()
{
    DenOfIz::Engine::Init();

/*    DenOfIz::ComputeTest computeTest;
    if (computeTest.Run() == 0) {
        return 0;
    }*/


#if defined WIN32 && defined DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
    int * n = new int[5];
    if ( SDL_Init(SDL_INIT_EVERYTHING) != 0 )
    {
        exit(1);
    }

    uint32_t windowFlags = SDL_WINDOW_SHOWN;
#if defined WIN32 || defined __linux__
    windowFlags |= SDL_WINDOW_VULKAN
#elif __APPLE__
    windowFlags |= SDL_WINDOW_METAL;
#endif

    auto window = SDL_CreateWindow("Hello C++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, windowFlags);

    auto windowHandle = std::make_unique<DenOfIz::GraphicsWindowHandle>();
    windowHandle->Create(window);
    { // SimpleRenderer scope
        auto renderer = DenOfIz::SimpleRenderer();
        renderer.Init(windowHandle.get());
        auto      running = true;
        SDL_Event event;
        while ( running )
        {
            while ( SDL_PollEvent(&event) )
            {
                if ( event.type == SDL_QUIT )
                {
                    running = false;
                }
            }

            renderer.Render();
        }
        renderer.Quit();
    }

    DenOfIz::GraphicsAPI::ReportLiveObjects();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
#endif

// #include <TF/Renderer/TriangleRenderer.h>
// DEFINE_APPLICATION_MAIN(DenOfIz::TriangleRenderer)
