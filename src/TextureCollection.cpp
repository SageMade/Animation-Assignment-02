#include "TextureCollection.h"

Texture2D TextureCollection::myTextures[255];

void TextureCollection::LoadTexture(uint8_t id, const char * filename) {
	if (myTextures[id - 1].id() != 0) {
		myTextures[id - 1].deleteTexture();
	}

	myTextures[id - 1].loadTextureFromFile(filename);
}

Texture2D TextureCollection::Get(uint8_t id) {
	return myTextures[id - 1];
}
