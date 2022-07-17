#include <SDL.h>
#include "helper.hpp"
#include "vector2.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <map>
#include <unordered_map>

struct Memory {
	void operator()(SDL_Window *x) { SDL_DestroyWindow(x); }
	void operator()(SDL_Renderer *x) { SDL_DestroyRenderer(x); }
};

using WNDPTR = std::unique_ptr<SDL_Window, Memory>;
using RNDRPTR = std::shared_ptr<SDL_Renderer>;

namespace lightning {
	RNDRPTR strike;
	uint32_t windowWidth, windowHeight;
}

using namespace gmtk;

struct Weapon {
	SDL_FRect collisionBox;
	int damage;
};

struct Sword : public Weapon {};

class Animation {
public:
	void addAnimation(std::string_view name, Texture spritesheet, int frames, int x, int y, int w, int h) {
		int width, height;
		SDL_QueryTexture(spritesheet.get(), nullptr, nullptr, &width, &height);

		// (i * w) + ((x / y) * 18)

		std::vector<SDL_Rect> rects;

		for (int i = 0; i < frames; ++i) {
			SDL_Rect newFrame = {0};
		//	if (x <= 0) {
				newFrame = {(i + x) * w, y, w, h};
		//	} else {
			//	newFrame = {(i * w) + ((x / y) * 18), y, w, h};
			//}

			rects.emplace_back(newFrame);
		}

		//for (int i = 0; i < frames; ++i) {
			// scroll on the x axis
			//SDL_Rect newFrame = {(i + x) * w, y, w, h};
			//rects.emplace_back(newFrame);
		//}

		this->frames.insert({name.data(), rects});

		this->spritesheet = spritesheet;
	}

	constexpr uint32_t getCurrentFrame() {
		return currentFrame;
	}

	void playAnimation(std::string_view animName, bool repeat) {
		repeatAnim = repeat;
		if (currentAnim != animName) {
			currentAnim = animName;
			currentFrame = 0;
		}
	}

	void setFrameSpeed(float speed) {
		frameDuration = speed;
	}

	void setScale(double x, double y) {
		spriteScalar = vec2d(x, y);
	}

	void update(float dt) {
		if (frames[currentAnim].size() > 0) {
			timeElapsed += static_cast<float>(dt);

			if (repeatAnim) {
				if (timeElapsed >= frameDuration) {
					timeElapsed = 0.0f;
					currentFrame = (currentFrame + 1) % static_cast<int>(frames[currentAnim].size());
				}
			} else {
				currentFrame = 0;
			}
		}
	}

	void draw(int x, int y) {
		SDL_Rect clip = frames[currentAnim][currentFrame];
		drawTexture(spritesheet.get(), lightning::strike.get(), x, y, &clip, spriteScalar.x, spriteScalar.y);
	}

public:
	bool repeatAnim;
	Texture spritesheet;
	float timeElapsed {0.0f};
	uint32_t currentFrame {0};
	float frameDuration {100.0f};
	std::basic_string<char> currentAnim;
	std::unordered_map<std::basic_string<char>, std::vector<SDL_Rect>> frames;
	vec2d spriteScalar {3, 3};
};

class Ladybug {
public:
	Ladybug() {
		anim = std::make_unique<Animation>();
		sprite = loadTexture("assets/ladybug.png", lightning::strike.get());
		anim->addAnimation("Attack", sprite, 7, 0, 0, 32, 27); // x 0, y 0, w 32, h 27
		anim->addAnimation("Idle", sprite, 2, 0, 27, 32, 27); // x 0, y 27, w 32, h 27
		anim->addAnimation("Dead", sprite, 1, 64, 27, 32, 27); // x 64, y 27, w 32, h 27
		anim->addAnimation("Run", sprite, 2, 96, 27, 32, 27); // x 224, y 27, w 32, h 27
		anim->playAnimation("Attack", true);
		//anim->setScale(3, 3);
		// add animation here
		UP = SDL_SCANCODE_W;
		DOWN = SDL_SCANCODE_S;
		LEFT = SDL_SCANCODE_A;
		RIGHT = SDL_SCANCODE_D;

		setPosition({(float)lightning::windowWidth / 2, (float)lightning::windowHeight / 2});
	}

	void setPosition(vec2f pos) { position = pos; }

	void draw() {
		anim->draw(position.x, position.y);
	}

	void update(float dt) {
		const uint8_t *keystate = SDL_GetKeyboardState(NULL);

		if (keystate[UP]) {
			if (keystate[LEFT]) {
				position.x -= 1.0f * dt;
				position.y -= 1.0f * dt;
			} else if (keystate[RIGHT]) {
				position.x += 1.0f * dt;
				position.y -= 1.0f * dt;
			} else {
				position.y -= 1.0f * dt;
			}
		}

		if (keystate[DOWN]) {
			if (keystate[LEFT]) {
				position.x -= 1.0f * dt;
				position.y += 1.0f * dt;
			} else if (keystate[RIGHT]) {
				position.x += 1.0f * dt;
				position.y += 1.0f * dt;
			} else {
				position.y += 1.0f * dt;
			}
		}

		if (!keystate[UP] && !keystate[DOWN] && !keystate[LEFT] && keystate[RIGHT]) {
			position.x += 1.0f * dt;
		}

		if (!keystate[UP] && !keystate[DOWN] && keystate[LEFT] && !keystate[RIGHT]) {
			position.x -= 1.0f * dt;
		}

		anim->update(dt);
	}

	void death() {}

	vec2f position;
	std::unique_ptr<Animation> anim;

private:
	SDL_Scancode UP, DOWN, LEFT, RIGHT;
	Texture sprite;
	int spriteWidth;
	int spriteHeight;
	Sword massiveFuckingSword;
};

int main(int, char **)
{
	SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);
	SDL_assert(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != 0);
	if (TTF_Init() == -1) return false;

	auto start = std::chrono::steady_clock::now();

	auto window = WNDPTR(SDL_CreateWindow("LADYBUGTHESLAYER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0));
	lightning::strike = RNDRPTR(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer);

	auto ladybug = std::make_unique<Ladybug>();
	ladybug->setPosition({100, 100});

	const double FPS = 240.0;
	const double delay = 1000.0 / FPS;

	SDL_Event ev;
	bool active = true;
	while (active) {
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
				case SDL_QUIT:
					active = false;
					break;

				case SDL_KEYDOWN: {
					switch (ev.key.keysym.sym) {
						case SDLK_b: {
							ladybug->anim->playAnimation("Idle", true);
							ladybug->anim->setFrameSpeed(200);
						} break;

						case SDLK_v: {
							ladybug->anim->playAnimation("Dead", false);
							ladybug->anim->setFrameSpeed(0);
						} break;

						case SDLK_m: {
							ladybug->anim->playAnimation("Attack", true);
							ladybug->anim->setFrameSpeed(100);
						} break;

						case SDLK_p: {
							ladybug->anim->playAnimation("Run", true);
							ladybug->anim->setFrameSpeed(150);
						} break;
					}
				} break;
			}
		}
		auto end = std::chrono::steady_clock::now();
		auto dt = std::chrono::duration<double, std::milli>(end - start);
		start = end;

		ladybug->update(dt.count());

		SDL_SetRenderDrawColor(lightning::strike.get(), 100, 100, 100, 255);
		SDL_RenderClear(lightning::strike.get());

		ladybug->draw();

		SDL_RenderPresent(lightning::strike.get());

		if (delay > dt.count())
			SDL_Delay(delay - dt.count());
	}

	SDL_Quit();

	return 0;
}