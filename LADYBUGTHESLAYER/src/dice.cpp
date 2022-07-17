#include <SDL.h>
#include "helper.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <random>

struct Memory {
	void operator()(SDL_Window *x) { SDL_DestroyWindow(x); }
	void operator()(SDL_Renderer *x) { SDL_DestroyRenderer(x); }
};

using WNDPTR = std::unique_ptr<SDL_Window, Memory>;
using RNDRPTR = std::shared_ptr<SDL_Renderer>;

namespace lightning {
	RNDRPTR strike;
	SDL_Point mousePos;
	static std::mt19937_64 gen(std::random_device {}());
}

using namespace gmtk;

class Dice {
public:
	Dice(int x, int y) : diceMin(x), diceMax(y) {
		tex = loadTexture("assets/dice.png", lightning::strike.get());
		auto dice1 = rollDice();
		diceText = loadTextOutline(std::to_string(dice1), lightning::strike.get(), "assets/Onest.ttf", {0, 0, 0}, 48);
		SDL_QueryTexture(tex.get(), nullptr, nullptr, &texWidth, &texHeight);
		box = {xpos, ypos, (float)texWidth, (float)texHeight};
	}

	int rollDice() { return dice(lightning::gen); }

	void draw() {
		drawTexture(tex.get(), lightning::strike.get(), xpos, ypos);
		drawTexture(diceText.get(), lightning::strike.get(), box.x, box.y);
	}

	void update() {
		box.x = xpos;
		box.y = ypos;
	}

	float xpos, ypos;

private:
	int diceMin, diceMax;
	std::uniform_int_distribution<int> dice {diceMin, diceMax};
	Texture tex;
	int texWidth;
	int texHeight;
	Texture diceText;
	SDL_FRect box;
};

std::vector<std::unique_ptr<Dice>> diceList;

int main(int, char **)
{
	SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);
	SDL_assert(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != 0);
	if (TTF_Init() == -1) return false;

	auto window = WNDPTR(SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0));
	lightning::strike = RNDRPTR(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer);

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

				case SDL_MOUSEBUTTONDOWN: {
				case SDL_BUTTON_LEFT: {
					auto dice = std::make_unique<Dice>(0, 50);
					dice->xpos = lightning::mousePos.x;
					dice->ypos = lightning::mousePos.y;
					diceList.push_back(std::move(dice));
				} break;
				} break;

				case SDL_MOUSEMOTION: {
					lightning::mousePos.x = ev.motion.x;
					lightning::mousePos.y = ev.motion.y;
				}
			}
		}
		auto end = std::chrono::steady_clock::now();
		auto dt = std::chrono::duration<double, std::milli>(end - start);
		start = end;

		SDL_SetRenderDrawColor(lightning::strike.get(), 0, 0, 0, 255);
		SDL_RenderClear(lightning::strike.get());

		for (const auto &i : diceList) {
			i->update();
			i->draw();
		}

		SDL_RenderPresent(lightning::strike.get());

		if (delay > dt.count())
			SDL_Delay(delay - dt.count());
	}

	SDL_Quit();

	return 0;
}