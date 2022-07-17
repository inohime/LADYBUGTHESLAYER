#include <SDL.h>
#include "helper.hpp"
#include "util.hpp"
#include "vector2.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <random>
#include <string>
//#include <unordered_map>
#include <map>

namespace gmtk {
	// this has no meaning, don't try to make sense of it.

	// things that currently work:
	/*
	dice
	player
	camera
	*/


	class Bullet;
	class Dice;

	namespace lightning {
		PTR<SDL_Renderer> strike;
		// test data below, don't keep here
		std::vector<std::unique_ptr<Bullet>> bullets;
		std::vector<std::unique_ptr<Dice>> dices;
		vec2f mousePos;
	}

	static std::mt19937_64 gen(std::random_device {}());

	// add user interface
	// add settings to do the following:
	// -fullscreen, -volume change (music, chunk)

	struct Button {
		SDL_FRect box {0};
		bool canQuit {false};
		bool isEnabled {false};
	};

	using BUTTONPTR = std::shared_ptr<Button>;

	class UInterface {
	public:

	private:
	};

	class Animation {
	public:
		void addAnimation(std::string_view name, Texture spritesheet, int frames, int x, int y, int w, int h) {
			for (int i = 0; i < frames; ++i) {
				SDL_Rect newFrame = {(i + x) * w, y, w, h};
				this->frames.insert({name.data(), newFrame});
			}

			this->spritesheet = spritesheet;
		}

		void update(float speed, double dt) {
			if (frames.size() > 0) {
				timeElapsed += static_cast<float>(dt);

				if (timeElapsed >= speed) {
					timeElapsed = 0.0f;
					currentFrame = (currentFrame + 1) % static_cast<int>(frames.size());
				}
			}
		}

		uint32_t getFrame(std::string_view frameName) {
			//uint32_t frameIndex = std::distance(frames.begin(), frames.end(), frames.find(frameName.data()));
		}

		void setCurrentFrame(std::string_view frameName) {}

	private:
		Texture spritesheet;
		float timeElapsed {0.0f};
		uint32_t currentFrame {0};
		std::map<std::basic_string<char>, SDL_Rect> frames;
	};

	class Dice;

	Dice rollSingleDice() {

	}

	// make several instances of these
	class Dice {
	public:
		Dice(int min, int max) : diceMin(min), diceMax(max) {
			diceSprite = loadTexture("assets/dice.png", lightning::strike.get());
			//auto dice1 = rollSingleDice();
			diceText = loadTextOutline(std::to_string(dice1), lightning::strike.get(), "assets/Onest.ttf", {0, 0, 0}, 48);
			SDL_QueryTexture(diceSprite.get(), nullptr, nullptr, &spriteWidth, &spriteHeight);
			box = {position.x, position.y, (float)spriteWidth, (float)spriteHeight};
			/*
			for (auto &i : diceSpriteList) {
				i = loadTexture("assets/dice.png", lightning::strike.get());

			}
			*/
		}
		// several dice types for modifiers
		/*
		int rollSingleDice() {return dice(gen);}

		std::pair<int, int> rollDualDice() {
			return {dice(gen), dice(gen)};
		}

		std::tuple<int, int, int> rollTripleDice() {
			return std::make_tuple(dice(gen), dice(gen), dice(gen));
		}

		std::tuple<int, int, int, int> rollQuadDice() {
			return std::make_tuple(dice(gen), dice(gen), dice(gen), dice(gen));
		}
		*/

		void draw(int x, int y) {
			position = vec2f(x, y);
			drawTexture(diceSprite.get(), lightning::strike.get(), position.x, position.y);
			drawTexture(diceText.get(), lightning::strike.get(), box.x, 50);
		}

		void update(float dt) {
			box.x = position.x;
			box.y = position.y;
		}

	private:
		int diceMin, diceMax;
		std::uniform_int_distribution<int> dice {diceMin, diceMax};
		Texture diceSprite;
		vec2f position;
		SDL_FRect box;
		Texture diceText;
		int spriteWidth;
		int spriteHeight;
		//std::vector<Texture> diceSpriteList;
		//std::vector<Texture> diceTextList;
	};

	class DiceModifier : public Dice {
	public:

	private:
	};

	class Entity {
	public:
		virtual void draw() {}
		virtual void update() {}

		// for clarity
		void setPosition(vec2f pos) { position = pos; }

		vec2f position;

	protected:
		std::basic_string<char> name;
		vec2f velocity;
		SDL_FRect collisionBox;
		Animation anim;
		Texture sprite;
		int spriteWidth;
		int spriteHeight;
		int HP;
	};

	class Weapon {
	public:
		virtual void draw() {}
		virtual void update() {}

		void setPosition(vec2f pos) { position = pos; }

		vec2f position;

	protected:
		vec2f velocity;
		SDL_FRect collisionBox;
		Texture sprite;
		int spriteWidth;
		int spriteHeight;
	};

	class AI : public Entity {
	public:

	private:
	};

	class Wasp;
	class Dragonfly;
	class Spider;
	class Frog;

	class Bullet : public Weapon {
	public:
		Bullet() {
			sprite = loadTexture("assets/particle.png", lightning::strike.get());
			SDL_QueryTexture(sprite.get(), nullptr, nullptr, &spriteWidth, &spriteHeight);
			double angle = std::atan2((double)lightning::mousePos.y - position.y, (double)lightning::mousePos.x - position.x);
			velocity = vec2f(1 * (float)std::cos(angle), 1 * (float)std::sin(angle));
			collisionBox = {position.x, position.y, (float)spriteWidth, (float)spriteHeight};
		}

		void draw(int x, int y) {
			drawTexture(sprite.get(), lightning::strike.get(), x, y);
		}

		void update(float dt) {
			position += vec2f(velocity.x, velocity.y);
			collisionBox.x = position.x;
			collisionBox.y = position.y;
		}

	private:
		friend class Wasp;
		friend class Dragonfly;
		friend class Spider;
		friend class Frog;

	private:
		float lifetime {0.0f};
	};

	class Sword : public Weapon {
	public:
		Sword() {}

		void draw(int x, int y) {}
		void update(float dt) {}

	private:
		int damage {0};
	};

	class Ladybug : public Entity {
	public:
		Ladybug() {
			sprite = loadTexture("assets/warrior.png", lightning::strike.get());
			// add animation here
			UP = SDL_SCANCODE_W;
			DOWN = SDL_SCANCODE_S;
			LEFT = SDL_SCANCODE_A;
			RIGHT = SDL_SCANCODE_D;
		}

		void draw() {
			drawTexture(sprite.get(), lightning::strike.get(), position.x, position.y);
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
		}

		void attack() {}

		void circleAttack() {}

	private:
		SDL_Scancode UP, DOWN, LEFT, RIGHT;
		Sword theChosenOne;
	};

	class Aphid : public Entity {
	public:
		Aphid() {}

		void draw(int x, int y) {}
		void update(float dt) {}

	private:
	};

	class Dragonfly : public Entity {
	public:
		Dragonfly() {}

		void draw(int x, int y) {}
		void update(float dt) {}

	private:
	};

	class Wasp : public Entity {
	public:
		Wasp() {
			sprite = loadTexture("assets/warrior.png", lightning::strike.get());
		}

		void draw(int x, int y) {
			drawTexture(sprite.get(), lightning::strike.get(), x, y);
		}

		void update(float dt) {}

	private:
	};

	class Spider : public Entity {
	public:
		Spider() {}

		void draw(int x, int y) {}
		void update(float dt) {}
	};

	class Frog : public Entity {
	public:
		Frog() {}

		void draw(int x, int y) {}
		void update(double dt) {}

	private:
	};

	static double distanceBetweenEntities(const Entity &e1, const Entity &e2) {
		double dist = std::abs(std::sqrt(std::pow(e2.position.x - e1.position.x, 2) + std::pow(e2.position.y - e1.position.y, 2)));
		return dist;
	}
}

using namespace gmtk;

int main(int, char **)
{
	SDL_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);
	SDL_assert(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != 0);
	if (TTF_Init() == -1) return false;

	auto begin = std::chrono::steady_clock::now();

	auto window = PTR<SDL_Window>(SDL_CreateWindow("Ladybug ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0));
	lightning::strike = PTR<SDL_Renderer>(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));

	auto ladybug = std::make_unique<Ladybug>();
	ladybug->setPosition({100, 100});
	/*
	auto diceTex = loadTexture("assets/dice.png", lightning::strike.get());

	auto dice = std::make_unique<Dice>();

	auto [ddice1, ddice2] = dice->rollDualDice();
	std::cout << ddice1 << ", " << ddice2 << '\n';

	auto [qdice1, qdice2, qdice3, qdice4] = dice->rollQuadDice();
	std::cout << qdice1 << ", " << qdice2 << ", " << qdice3 << ", " << qdice4 << '\n';
	*/
	auto dice = std::make_unique<Dice>(5, 10);

	auto wasp = std::make_unique<Wasp>();

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

				case SDL_MOUSEBUTTONDOWN: {
				case SDL_BUTTON_LEFT: {
					auto nb = std::make_unique<Bullet>();
					lightning::bullets.push_back(std::move(nb));
					std::cout << lightning::bullets.size() << '\n';
				} break;
				} break;

				case SDL_MOUSEMOTION:
					lightning::mousePos = vec2f((float)ev.motion.x, (float)ev.motion.y);
					break;
			}
		}
		auto end = std::chrono::steady_clock::now();
		auto dt = std::chrono::duration<double, std::milli>(end - begin);
		begin = end;

		ladybug->update(static_cast<float>(dt.count()));
		dice->update(static_cast<float>(dt.count()));
		//wasp->update(static_cast<float>(dt.count()), &ev);

		SDL_SetRenderDrawColor(lightning::strike.get(), 0, 0, 0, 255);
		SDL_RenderClear(lightning::strike.get());

		//wasp->draw(100, 100);

		for (auto &bullet : lightning::bullets) {
			bullet->draw(bullet->position.x, bullet->position.y);
			bullet->update(static_cast<float>(dt.count()));
		}

		ladybug->draw();

		dice->draw(500, 50);

		SDL_RenderPresent(lightning::strike.get());

		if (delay > dt.count())
			SDL_Delay(static_cast<uint32_t>(delay - dt.count()));
	}

	SDL_Quit();

	return 0;
}