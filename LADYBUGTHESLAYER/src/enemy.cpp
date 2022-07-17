// make the enemy follow the player

#include <SDL.h>
#include <iostream>
#include "vector2.hpp"
#include <memory>
#include <chrono>

struct Memory {
	void operator()(SDL_Window *x) { SDL_DestroyWindow(x); }
	void operator()(SDL_Renderer *x) { SDL_DestroyRenderer(x); }
};

using WNDPTR = std::unique_ptr<SDL_Window, Memory>;
using RNDRPTR = std::unique_ptr<SDL_Renderer, Memory>;

namespace lightning {
	RNDRPTR strike;
	uint32_t w {1024}, h {768};
}

static double distanceBetweenEntities(const SDL_Rect &e1, const SDL_Rect &e2) {
	double dist = std::abs(std::sqrt(std::pow(e2.x - e1.x, 2) + std::pow(e2.y - e1.y, 2)));
	return dist;
}

double mlerp(double a, double b, double t)
{
	return (1 - t) * a + t * b;
}

using namespace gmtk;

// credit unity
vec2d moveTowards(vec2d current, vec2d target, float maxDistDelta) {
	vec2d a = target - current;

	float mag = std::abs(std::sqrt(std::pow(current.x - target.x, 2) + std::pow(current.y - target.y, 2)));

	if (mag <= maxDistDelta || mag == 0.f) {
		return target;
	}

	double x = current.x + a.x / mag * maxDistDelta;
	double y = current.y + a.y / mag * maxDistDelta;

	return vec2d(x, y);
}

int main(int, char **)
{
	SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);

	auto start = std::chrono::steady_clock::now();

	auto window = WNDPTR(SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0));
	lightning::strike = RNDRPTR(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));

	SDL_Rect pp = {lightning::w / 2, lightning::h / 2, 25, 25};
	SDL_Rect ep = {0, 0, 25, 25};

	const double FPS = 240.0;
	const double delay = 1000.0 / FPS;

	float x = 0.f;
	float y = 0.f;

	SDL_Event ev;
	bool active = true;
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

		const uint8_t *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_W]) {
			pp.y -= (int)(1 * dt.count());
		}

		if (keys[SDL_SCANCODE_A]) {
			pp.x -= (int)(1 * dt.count());
		}

		if (keys[SDL_SCANCODE_S]) {
			pp.y += (int)(1 * dt.count());
		}

		if (keys[SDL_SCANCODE_D]) {
			pp.x += (int)(1 * dt.count());
		}

		float dist = distanceBetweenEntities(pp, ep);

		std::cout << ep.x << ", " << ep.y << '\n';

		vec2d ep2d = moveTowards(vec2d(ep.x, ep.y), vec2d(pp.x, pp.y), (1 * dt.count()) / 2);

		ep.x = ep2d.x;
		ep.y = ep2d.y;

		SDL_SetRenderDrawColor(lightning::strike.get(), 0, 0, 0, 255);
		SDL_RenderClear(lightning::strike.get());

		SDL_SetRenderDrawColor(lightning::strike.get(), 255, 0, 0, 255);
		SDL_RenderFillRect(lightning::strike.get(), &ep);

		SDL_SetRenderDrawColor(lightning::strike.get(), 255, 255, 255, 255);
		SDL_RenderFillRect(lightning::strike.get(), &pp);

		SDL_RenderPresent(lightning::strike.get());

		if (delay > dt.count())
			SDL_Delay(delay - dt.count());
	}

	SDL_Quit();

	return 0;
}