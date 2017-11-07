#include "TextureCollection.h"

#include <sstream>

#include "FileHelpers.h"

Texture2D TextureCollection::myTextures[255];

void TextureCollection::LoadTexture(uint8_t id, const char * filename) {
	if (myTextures[id].id() != 0) {
		myTextures[id].deleteTexture();
	}

	myTextures[id].loadTextureFromFile(filename);
}

Texture2D& TextureCollection::Get(uint8_t id) {
	return myTextures[id];
}

void TextureCollection::WriteToFile(std::fstream& stream) {
	uint8_t count = 0;
	std::streampos pos = stream.tellg();
	Write(stream, count);
	for (int ix = 0; ix < 255; ix++) {
		if (myTextures[ix].id() != 0) {
			count++;
			Write(stream, (uint8_t)ix);
			myTextures[ix].writeToFile(stream);
		}
	}
	std::streampos endPos = stream.tellg();
	stream.seekg(pos);
	Write(stream, count);
	stream.seekg(endPos);
	endPos = stream.tellg();
}

void TextureCollection::ReadFromFile(std::fstream & stream) {
	uint8_t count = 0;
	Read(stream, count);
	uint8_t location = 0;
	for (int ix = 0; ix < count; ix++) {
		Read(stream, location);
		myTextures[location].readFromFile(stream);
	}
}
