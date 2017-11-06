#pragma once

#include "TTK/Texture2D.h"

class TextureCollection {
public:
	static void LoadTexture(uint8_t id, const char* filename);
	static const Texture2D& Get(uint8_t id);


private:
	static Texture2D myTextures[255];
};