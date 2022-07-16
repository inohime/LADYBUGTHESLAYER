#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <memory>

namespace gmtk {
	using Texture = std::shared_ptr<SDL_Texture>;

	Texture loadTexture(std::string_view filePath, SDL_Renderer *ren, SDL_Color *key = nullptr) {
		SDL_Surface *surf = IMG_Load(filePath.data());
		if (surf == nullptr) {
			std::cout << "Failed to load texture \n";
			return nullptr;
		}

		if (key != nullptr)
			SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGBA(surf->format, key->r, key->g, key->b, key->a));

		auto tex = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(ren, surf), SDL_DestroyTexture);
		if (tex.get() == nullptr) {
			std::cout << "Text texture failed to be created\n";
		}

		SDL_FreeSurface(surf);

		return tex;
	}

	Texture loadText(std::string_view msg, SDL_Renderer *ren, std::string_view fontFile, const SDL_Color &col, int fontSize) {
		TTF_Font *font = TTF_OpenFont(fontFile.data(), fontSize);
		if (font == nullptr) {
			std::cout << "TTF_OpenFont error " << TTF_GetError() << "\n";
			return nullptr;
		}

		SDL_Surface *surf = TTF_RenderText_Blended(font, msg.data(), col);
		if (surf == nullptr) {
			TTF_CloseFont(font);
			std::cout << "TTF_RenderText error " << TTF_GetError() << "\n";
			return nullptr;
		}

		auto tex = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(ren, surf), SDL_DestroyTexture);
		if (tex.get() == nullptr) {
			std::cout << "Text texture failed to be created\n";
		}

		SDL_FreeSurface(surf);
		TTF_CloseFont(font);

		return tex;
	}

	Texture loadTextOutline(std::string_view msg, SDL_Renderer *ren, std::string_view fontFile, const SDL_Color &col, int fontSize) {
		TTF_Font *font = TTF_OpenFont(fontFile.data(), fontSize);
		if (font == nullptr) {
			std::cout << "TTF_OpenFont error " << TTF_GetError() << "\n";
			return nullptr;
		}

		TTF_Font *outlineFont = TTF_OpenFont(fontFile.data(), fontSize);
		if (font == nullptr) {
			std::cout << "TTF_OpenFont error " << TTF_GetError() << "\n";
			return nullptr;
		}

		TTF_SetFontOutline(outlineFont, 1);

		// background | foreground
		SDL_Surface *bgSurf = TTF_RenderText_Blended(font, msg.data(), col);
		SDL_Surface *fgSurf = TTF_RenderText_Blended(outlineFont, msg.data(), {0x00, 0x00, 0x00});

		// destination rect that gets the size of the surface (explicit x/y for those that want to understand without digging)
		SDL_Rect position = {position.x = 1, position.y = 1, fgSurf->w, fgSurf->h};
		SDL_BlitSurface(bgSurf, nullptr, fgSurf, &position);

		auto tex = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(ren, fgSurf), SDL_DestroyTexture);
		if (tex.get() == nullptr) {
			std::cout << "Text texture failed to be created\n";
			return nullptr;
		}

		SDL_FreeSurface(bgSurf);
		SDL_FreeSurface(fgSurf);
		TTF_CloseFont(outlineFont);
		TTF_CloseFont(font);

		return tex;
	}

	void drawCircle(SDL_Renderer *ren, float x, float y, float radius) {
		constexpr int tris = 225; // amount of triangles
		float mirror = 2.0f * static_cast<float>(M_PI); // get the other half of the circle 
		SDL_Vertex vertices[tris] = {0};

		for (int i = 0; i < tris; i += 3) {
			// the upper bound of the triangle
			vertices[i].position.x = x; // 0, 3, 6, 9 
			vertices[i].position.y = y;
			vertices[i].color = {255, 255, 255, 255};

			// subtract 3 from tris so we don't operate on a triangle that is out of bounds

			// the lower right bound of the triangle
			vertices[i + 1].position.x = x + (std::cos(i * mirror / (tris - 3)) * radius); // 1, 4, 7, 10
			vertices[i + 1].position.y = y + (std::sin(i * mirror / (tris - 3)) * radius);
			vertices[i + 1].color = {255, 255, 255, 255};

			// the lower left bound of the triangle
			if (i > 0) {
				vertices[i - 1].position.x = x + (std::cos(i * mirror / (tris - 3)) * radius); // 2, 5, 8, 11
				vertices[i - 1].position.y = y + (std::sin(i * mirror / (tris - 3)) * radius);
				vertices[i - 1].color = {255, 255, 255, 255};
			}
		}

		SDL_RenderGeometry(ren, NULL, vertices, tris - 3, NULL, tris - 3);
	}

	void drawGlow(SDL_Renderer *ren, float x, float y, float radius) {
		constexpr int tris = 225; // amount of triangles
		float mirror = 2.0f * static_cast<float>(M_PI); // get the other half of the circle 
		SDL_Vertex vertices[tris] = {0};

		for (int i = 0; i < tris; i += 3) {
			// the upper bound of the triangle
			vertices[i].position.x = x; // 0, 3, 6, 9 
			vertices[i].position.y = y;
			vertices[i].color = {255, 255, 255, 255};

			// subtract 3 from tris so we don't operate on a triangle that is out of bounds

			// the lower right bound of the triangle
			vertices[i + 1].position.x = x + (std::cos(i * mirror / (tris - 3)) * radius); // 1, 4, 7, 10
			vertices[i + 1].position.y = y + (std::sin(i * mirror / (tris - 3)) * radius);

			// the lower left bound of the triangle
			if (i > 0) {
				vertices[i - 1].position.x = x + (std::cos(i * mirror / (tris - 3)) * radius); // 2, 5, 8, 11
				vertices[i - 1].position.y = y + (std::sin(i * mirror / (tris - 3)) * radius);
			}
		}

		SDL_RenderGeometry(ren, NULL, vertices, tris - 3, NULL, tris - 3);
	}

	void drawTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr, double sx = 0.0, double sy = 0.0) noexcept {
		SDL_Rect dst = {};
		dst.x = x;
		dst.y = y;
		if (clip != nullptr) {
			dst.w = clip->w;
			dst.h = clip->h;
		} else {
			SDL_QueryTexture(tex, nullptr, nullptr, &dst.w, &dst.h);
		}

		if ((sx && sy) != NULL) {
			dst.w *= static_cast<int>(sx);
			dst.h *= static_cast<int>(sy);
		}

		SDL_RenderCopy(ren, tex, clip, &dst);
	}

	void drawTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_RendererFlip flip, SDL_Rect *clip = nullptr, double angle = 0.0, SDL_Point *center = nullptr) noexcept {
		SDL_Rect dst = {};
		dst.x = x;
		dst.y = y;
		if (clip != nullptr) {
			dst.w = clip->w;
			dst.h = clip->h;
		} else {
			SDL_QueryTexture(tex, nullptr, nullptr, &dst.w, &dst.h);
		}

		SDL_RenderCopyEx(ren, tex, clip, &dst, angle, center, flip);
	}

	template <typename T>
	T *loadSound(std::string_view fileName) {
		T *sound = nullptr;
		if constexpr (std::is_same_v<T, Mix_Music>) {
			sound = Mix_LoadMUS(fileName.data());
			if (sound == nullptr) {
				std::cout << "Failed to load path\n";
			} else {
				std::cout << "Loaded path, preparing music..\n";
			}
		} else if constexpr (std::is_same_v<T, Mix_Chunk>) {
			sound = Mix_LoadWAV(fileName.data());
			if (sound == nullptr) {
				std::cout << "Failed to load path\n";
			} else {
				std::cout << "Loaded path, preparing chunk samples..\n";
			}
		}
		return sound;
	}

	template <typename T>
	void playSound(T *sound) noexcept {
		if constexpr (std::is_same_v<T, Mix_Music>) {
			Mix_PlayMusic(sound, 0);
			if (sound == nullptr) {
				std::cout << "Failed to play music\n";
			}
		} else if constexpr (std::is_same_v<T, Mix_Chunk>) {
			Mix_PlayChannel(-1, sound, NULL);
			if (sound == nullptr) {
				std::cout << "Failed to play chunk\n";
			}
		}
	}
} // namespace gmtk