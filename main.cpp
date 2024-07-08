#ifndef DISABLED
// Include first to ensure NOMINMAX
#include <DenOfIzCore/Engine.h>

#define SDL_MAIN_HANDLED
#include <DenOfIzGraphics/Renderer/SimpleRenderer.h>
#include <SDL2/SDL.h>
#include <filesystem>

void BitSetTest()
{

}

int main()
{
    DenOfIz::Engine::Init();

#if defined WIN32 && defined DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
    int * n = new int[5];
    if ( SDL_Init(SDL_INIT_EVERYTHING) != 0 )
    {
        exit(1);
    }

    auto window = SDL_CreateWindow("Hello C++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

    auto renderer = DenOfIz::SimpleRenderer();
    auto windowHandle = std::make_unique<DenOfIz::GraphicsWindowHandle>();
    windowHandle->Create(window);
    renderer.Init(windowHandle.get());

    auto running = true;
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
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
#endif

// #include <TF/Renderer/TriangleRenderer.h>
// DEFINE_APPLICATION_MAIN(DenOfIz::TriangleRenderer)
