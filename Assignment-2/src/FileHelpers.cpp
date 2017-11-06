#include "FileHelpers.h"

void Write(std::fstream & stream, void * data, size_t size) {
	stream.write(reinterpret_cast<char*>(data), size);
}

void Read(std::fstream & stream, void * data, size_t size) {
	stream.read(reinterpret_cast<char*>(data), size);
}
