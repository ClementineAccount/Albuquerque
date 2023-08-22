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
		int pos_x = 0;
		int pos_y = 0;
		int size = 10;
		Uint8 r = 0xFF;
		Uint8 g = 0x00;
		Uint8 b = 0x00;
		Uint8 a = 0xFF;

	};

	Block block{ 10, 10 };
	Block bigBlock{ 30, 20, 100, 0x00, 0x00, 0xFF, 0xFF };

	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	uint32_t prevFrame = 0;
	//Uint64 currFrame = 0;
	uint32_t deltaTime = 0;

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
			uint32_t currFrame = SDL_GetTicks64();
			deltaTime = currFrame - prevFrame;
			prevFrame = currFrame;

			SDL_PollEvent(&e);

			if (e.type == SDL_QUIT) 
				quit = true;

			//deltaTime = SDL_GetTicks() - deltaTime;
			//printf("%f\n", deltaTime);


			static float speed = 1.0f;

			static bool isLeft = false;
			isLeft = false;
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_LEFT)
				{
					block.pos_x -= 1 * deltaTime;
				}
				else if (e.key.keysym.sym == SDLK_RIGHT)
				{
					block.pos_x += 1 * deltaTime;
				}
			}

			block.pos_y += deltaTime * 0.1f;

			auto checkBlockIntersection = [&](const Block& lhs, const Block& rhs)
			{
				SDL_Rect lhs_rect = { lhs.pos_x, lhs.pos_y, lhs.size, lhs.size };
				SDL_Rect rhs_rect = { rhs.pos_x, rhs.pos_y, rhs.size, rhs.size };
				return SDL_HasIntersection(&lhs_rect, &rhs_rect);
			};

			//Set the big block to green if red block enters it
			if (checkBlockIntersection(block, bigBlock))
			{
				bigBlock.b = 0xFF;
				bigBlock.g = 255;
			}


			static SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(renderer);

			auto drawBlock = [&](const Block& block)
			{
				SDL_Rect fillRect = { block.pos_x, block.pos_y, block.size, block.size };
				SDL_SetRenderDrawColor(renderer, block.r, block.g, block.b, block.a);
				SDL_RenderFillRect(renderer, &fillRect);
			};

			drawBlock(block);
			drawBlock(bigBlock);

			SDL_RenderPresent(renderer);
		}
	}

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
