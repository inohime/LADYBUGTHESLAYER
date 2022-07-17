#include <SDL.h>
#include "helper.hpp"
#include "vector2.hpp"
#include <iostream>
#include <memory>
#include <chrono>

using namespace gmtk;

struct Memory {
	void operator()(SDL_Window *x) { SDL_DestroyWindow(x); }
	void operator()(SDL_Renderer *x) { SDL_DestroyRenderer(x); }
};

using WNDPTR = std::unique_ptr<SDL_Window, Memory>;
using RNDRPTR = std::unique_ptr<SDL_Renderer, Memory>;

namespace lightning {
	RNDRPTR strike;
	uint32_t w {1280}, h {720};
}

struct Wall {
	Wall() {
		wallTex = loadTexture("assets/rock.png", lightning::strike.get());
	}

	void draw() {
		drawTexture(wallTex.get(), lightning::strike.get(), pos.x, pos.y);
	}

	//void update() {

	//}

	Texture wallTex;
	vec2f pos;
	SDL_FRect box;
};

int main(int, char **)
{
	SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);

	int screenW = 1280, screenH = 720;

	auto window = WNDPTR(SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenW, screenH, 0));
	lightning::strike = RNDRPTR(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));
	auto target = SDL_CreateTexture(lightning::strike.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, screenW, screenH);

	Texture map = loadTexture("assets/map.png", lightning::strike.get());

	SDL_Rect makeTextureBig = {0, 0, screenW, screenH};
	SDL_FRect pp = {screenW / 2, screenH / 2, 35, 40};
	vec2f lastPos;
	vec2f ppPos = vec2f(screenW / 2, screenH / 2);

	std::vector<std::unique_ptr<Wall>> walls;
	int tileSize = 32;

	for (int i = 0; i < screenW / tileSize; i++) {
		// top row
		auto nwt = std::make_unique<Wall>();
		nwt->pos = vec2f(i * tileSize, 0);
		nwt->box = {nwt->pos.x, nwt->pos.y, (float)tileSize, (float)tileSize};
		walls.push_back(std::move(nwt));

		// bottom row
		auto nwb = std::make_unique<Wall>();
		nwb->pos = vec2f(i * tileSize, screenH - tileSize);
		nwb->box = {nwb->pos.x + makeTextureBig.x, nwb->pos.y, (float)tileSize, (float)tileSize};
		walls.push_back(std::move(nwb));
	}

	for (int j = 0; j < screenH / tileSize; j++) {
		// right column
		auto nwr = std::make_unique<Wall>();
		nwr->pos = vec2f(screenW - tileSize, j * tileSize);
		nwr->box = {nwr->pos.x, nwr->pos.y, (float)tileSize, (float)tileSize};
		printf("right column pos: %f, %f\n", nwr->box.x, nwr->box.y);
		walls.push_back(std::move(nwr));
		
		// left column
		auto nwl = std::make_unique<Wall>();
		nwl->pos = vec2f(0, j * tileSize);
		nwl->box = {nwl->pos.x, nwl->pos.y, (float)tileSize, (float)tileSize};
		walls.push_back(std::move(nwl));
	}

	std::cout << walls.size() << '\n';

	const double FPS = 240.0;
	const double delay = 1000.0 / FPS;

	SDL_Event ev;
	bool active = true;
	auto start = std::chrono::steady_clock::now();
	while (active) {
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
				case SDL_QUIT:
					active = false;
					break;
			}
		}
		auto end = std::chrono::steady_clock::now();
		auto dt = std::chrono::duration<double, std::milli>(end - start);
		start = end;

		lastPos = ppPos;

		const uint8_t *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_W]) {
			//pp.y -= (int)(1 * dt.count());
			ppPos.y -= (int)(1 * dt.count());
			//makeTextureBig.y += 1 * dt.count();
		}

		if (keys[SDL_SCANCODE_A]) {
			//pp.x -= (int)(1 * dt.count());
			ppPos.x -= (int)(1 * dt.count());
			//makeTextureBig.x += 1 * dt.count();
		}

		if (keys[SDL_SCANCODE_S]) {
			//pp.y += (int)(1 * dt.count());
			ppPos.y += (int)(1 * dt.count());
			//makeTextureBig.y -= 1 * dt.count();
		}

		if (keys[SDL_SCANCODE_D]) {
			//pp.x += (int)(1 * dt.count());
			ppPos.x += (int)(1 * dt.count());
			//makeTextureBig.x -= 1 * dt.count();
		}

		SDL_SetRenderDrawColor(lightning::strike.get(), 0, 0, 0, 255);
		SDL_RenderClear(lightning::strike.get());

		SDL_Rect dst = {0, 0, screenW, screenH};

		pp = {ppPos.x, ppPos.y, 35, 40};
		

		SDL_SetRenderTarget(lightning::strike.get(), target);
		SDL_RenderCopy(lightning::strike.get(), map.get(), &dst, nullptr);
		SDL_SetRenderTarget(lightning::strike.get(), nullptr);

		drawTexture(target, lightning::strike.get(), makeTextureBig.x, makeTextureBig.y);

		printf("%f, %f\n", pp.x, pp.y);

		for (const auto &wall : walls) {

			auto collide = SDL_HasIntersectionF(&pp, &wall->box);

			// set walls to stick to the target texture

			wall->draw();
			
			if (!collide) {
				SDL_SetRenderDrawColor(lightning::strike.get(), 255, 0, 0, 255);
				SDL_RenderDrawRectF(lightning::strike.get(), &wall->box);
				//lastPos = ppPos;
				//lastPos = vec2f(pp.x, pp.y);

			} else {
				SDL_SetRenderDrawColor(lightning::strike.get(), 0, 255, 0, 255);
				SDL_RenderDrawRectF(lightning::strike.get(), &wall->box);
				//printf("colliding\n");
				//vec2f(pp.x, pp.y) = lastPos;
				ppPos = lastPos;
				//ppPos = vec2f(0, 0);
			}
			
		}

		SDL_SetRenderDrawColor(lightning::strike.get(), 255, 0, 0, 255);
		SDL_RenderFillRectF(lightning::strike.get(), &pp);

		SDL_RenderPresent(lightning::strike.get());

		if (delay > dt.count())
			SDL_Delay(delay - dt.count());

		printf("%f, %f\n", lastPos.x, lastPos.y);

	}

	SDL_DestroyTexture(target);
	SDL_Quit();

	return 0;
}