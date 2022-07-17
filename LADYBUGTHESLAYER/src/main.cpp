#include <SDL.h>
#include "helper.hpp"
#include "util.hpp"
#include "vector2.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <random>
#include <string>
#include <unordered_map>
#include <map>
#include <vector>

/* LAYOUT
x. MAP
x. WALLS
x. DICE
x. ANIMATION
x. PLAYER
x. ENEMIES
x. ATTACKS
x. HUD
*/

namespace gmtk {
	class Bullet;
	class Dice;
	class Wall;
	class Wasp;
	class Spider;
	class Frog;

	namespace lightning {
		PTR<SDL_Renderer> strike;
		vec2f mousePos;
		
		//std::vector<std::unique_ptr<Bullet>> bullets;
		//std::vector<std::unique_ptr<Dice>> dices;
		std::vector<std::unique_ptr<Wall>> walls;
	}

	class Wall {
	public:
		Wall() {
			wallTex = loadTexture("assets/wall.png", lightning::strike.get());
		}

		void draw() {
			drawTexture(wallTex.get(), lightning::strike.get(), pos.x, pos.y);
		}

	public:
		Texture wallTex;
		vec2f pos;
		SDL_FRect box;
	};

	class Animation {
	public:
		void addAnimation(std::string_view name, Texture spritesheet, int frames, int x, int y, int w, int h) {
			int width, height;
			SDL_QueryTexture(spritesheet.get(), nullptr, nullptr, &width, &height);

			std::vector<SDL_Rect> rects;

			for (int i = 0; i < frames; ++i) {
				SDL_Rect newFrame = {0};
				newFrame = {(i + x) * w, y, w, h};
				rects.emplace_back(newFrame);
			}

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

	class Entity {
	public:
		virtual void draw() {}
		virtual void update() {}
		void setPosition(vec2f pos) { position = pos; }
		bool hasIntersection(Entity &e1, Entity &e2) {
			return SDL_HasIntersectionF(&e1.box, &e2.box);
		}
		
		bool hasIntersection(Entity &e1, Wall &w1) {
			return SDL_HasIntersectionF(&e1.box, &w1.box);
		}

		vec2f position;

	protected:
		std::basic_string<char> name;
		vec2f velocity;
		SDL_FRect box;
		Animation anim;
		Texture sprite;
		int spriteWidth;
		int spriteHeight;
		int HP;
	};

	class Bullet {
	public:
		Bullet(vec2f &epos) {
			sprite = loadTexture("assets/particle.png", lightning::strike.get());
			SDL_QueryTexture(sprite.get(), nullptr, nullptr, &spriteWidth, &spriteHeight);
			double angle = std::atan2((double)epos.y - position.y, (double)epos.x - position.x);
			velocity = vec2f(bulletSpeed * (float)std::cos(angle), bulletSpeed * (float)std::sin(angle));
			box = {position.x, position.y, (float)spriteWidth, (float)spriteHeight};
		}

		void draw(int x, int y) {
			drawTexture(sprite.get(), lightning::strike.get(), x, y);
		}

		void update(float dt) {
			position += vec2f(velocity.x, velocity.y);
			box.x = position.x;
			box.y = position.y;
		}

		vec2f position;

	private:
		friend class Wasp;
		friend class Dragonfly;
		friend class Spider;
		friend class Frog;

	private:
		float lifetime {0.0f};
		vec2f velocity;
		SDL_FRect box;
		Texture sprite;
		int spriteWidth;
		int spriteHeight;
		int bulletSpeed;
	};
} // namespace gmtk

using namespace gmtk;

int main(int, char **)
{
	constexpr int screenW = 1280, screenH = 720;

	SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);
	SDL_assert(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != 0);
	if (TTF_Init() == -1) return false;

	auto begin = std::chrono::steady_clock::now();

	auto window = PTR<SDL_Window>(SDL_CreateWindow("LADYBUGTHESLAYER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenW, screenH, 0));
	lightning::strike = PTR<SDL_Renderer>(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));

	auto background = loadTexture("assets/map.png", lightning::strike.get());

	int tileSize = 32;
	for (int i = 0; i < screenW / tileSize; i++) {
		// top row
		auto nwt = std::make_unique<Wall>();
		nwt->pos = vec2f(i * tileSize, 0);
		nwt->box = {nwt->pos.x, nwt->pos.y, (float)tileSize, (float)tileSize};
		lightning::walls.push_back(std::move(nwt));

		// bottom row
		auto nwb = std::make_unique<Wall>();
		nwb->pos = vec2f(i * tileSize, screenH - tileSize);
		nwb->box = {nwb->pos.x, nwb->pos.y, (float)tileSize, (float)tileSize};
		lightning::walls.push_back(std::move(nwb));
	}

	for (int j = 0; j < screenH / tileSize; j++) {
		// right column
		auto nwr = std::make_unique<Wall>();
		nwr->pos = vec2f(screenW - tileSize, j * tileSize);
		nwr->box = {nwr->pos.x, nwr->pos.y, (float)tileSize, (float)tileSize};
		lightning::walls.push_back(std::move(nwr));

		// left column
		auto nwl = std::make_unique<Wall>();
		nwl->pos = vec2f(0, j * tileSize);
		nwl->box = {nwl->pos.x, nwl->pos.y, (float)tileSize, (float)tileSize};
		lightning::walls.push_back(std::move(nwl));
	}

	const double FPS = 72.0;
	const double delay = 1000.0 / FPS;

	SDL_Event ev;
	bool active = true;
	while (active) {
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
				case SDL_QUIT:
					active = false;
					break;

				case SDL_MOUSEBUTTONDOWN: {
					//case SDL_BUTTON_LEFT: {
						 // for attacking
					//} break;
				} break;

				case SDL_MOUSEMOTION:
					//lightning::mousePos = vec2f((float)ev.motion.x, (float)ev.motion.y);
					break;
			}
		}
		auto end = std::chrono::steady_clock::now();
		auto dt = std::chrono::duration<double, std::milli>(end - begin);
		begin = end;

		SDL_SetRenderDrawColor(lightning::strike.get(), 0, 0, 0, 255);
		SDL_RenderClear(lightning::strike.get());

		drawTexture(background.get(), lightning::strike.get(), 0, 0);

		for (const auto &wall : lightning::walls) {
			// check collision for all entities
			//auto collide = SDL_HasIntersectionF(&, &wall->box);
			wall->draw();
		}

		SDL_RenderPresent(lightning::strike.get());

		if (delay > dt.count())
			SDL_Delay(static_cast<uint32_t>(delay - dt.count()));
	}

	SDL_Quit();

	return 0;
}