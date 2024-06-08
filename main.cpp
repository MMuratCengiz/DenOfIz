#ifdef DISABLED
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <filesystem>
#include <DenOfIzGraphics/Renderer/SimpleRenderer.h>

int main()
{
#if defined WIN32 && defined DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		exit(1);
	}

	auto window = SDL_CreateWindow(
			"Hello C++",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			800,
			600,
			SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN
	);

	auto renderer = DenOfIz::SimpleRenderer();
	renderer.Init(window);

	auto running = true;
	SDL_Event event;
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
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

//#include <IApp.h>
#include <TF/Renderer/TriangleRenderer.h>

//DEFINE_APPLICATION_MAIN(RaytracingApp);
//DEFINE_APPLICATION_MAIN(MainApp)
DEFINE_APPLICATION_MAIN(DenOfIz::TriangleRenderer)