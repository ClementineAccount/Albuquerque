/*This source code copyrighted by Lazy Foo' Productions 2004-2023
and may not be redistributed without written permission.*/

//I am just using this for the Hello World its gonna look really different really fast Mr Lazy Foo... (https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php)

//Using SDL and standard IO
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[])
{

	struct Block
	{
		float pos_x = 0;
		float pos_y = 0;
		uint32_t size = 10;
	};

	Block block{ 10, 10 };

	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	Uint64 prevFrame = SDL_GetTicks64();
	Uint64 currFrame = SDL_GetTicks64();
	double deltaTime = (currFrame - prevFrame) / 1000.0f;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("Yo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		SDL_Event e; 
		bool quit = false; 
		while (quit == false)
		{
			currFrame = SDL_GetTicks64();
			deltaTime = (static_cast<double>(currFrame) - static_cast<double>(prevFrame)) / 1000.0;
			prevFrame = currFrame;

			SDL_PollEvent(&e);

			if (e.type == SDL_QUIT) 
				quit = true;

			//deltaTime = SDL_GetTicks() - deltaTime;
			//printf("%f\n", deltaTime);

			static float q speed = 15;
			block.pos_x += static_cast<float>(deltaTime) * speed;
			block.pos_y += static_cast<float>(deltaTime) * speed;

			static SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(renderer);

			SDL_Rect fillRect = { block.pos_x, block.pos_y, block.size, block.size };
			SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
			SDL_RenderFillRect(renderer, &fillRect);

			SDL_RenderPresent(renderer);
		}
	}

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
