#pragma once

#include <fstream>

template <typename T>
void Write(std::fstream& stream, T *data) {
	stream.write(reinterpret_cast<char*>(data), sizeof(T));
}

void Write(std::fstream& stream, void *data, size_t size);

template <typename T>
void Read(std::fstream& stream, T *data) {
	stream.read(reinterpret_cast<char*>(data), sizeof(T));
}

void Read(std::fstream& stream, void *data, size_t size);