/*
	Authors:
	Shawn M.          100412327
	Paul Puig         100656910
	Stephen Richards  100458273
*/
#pragma once

#include "TTK/Texture2D.h"

#include <fstream>

class TextureCollection {
	public:
		static void LoadTexture(uint8_t id, const char* filename);
		static Texture2D& Get(uint8_t id);

		static void WriteToFile(std::fstream& stream);
		static void ReadFromFile(std::fstream& stream);

	private:
		static Texture2D myTextures[255];
};