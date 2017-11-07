#pragma once

#include <fstream>

template <typename T>
void Write(std::fstream& stream, const T& data) {
	stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
}

template <typename T>
void Write(std::fstream& stream, const T& data, size_t size) {
	stream.write(reinterpret_cast<const char*>(&data), size);
}

template <typename T>
void Write(std::fstream& stream, const T* data) {
	stream.write(reinterpret_cast<const char*>(data), sizeof(T));
}

template <typename T>
void Write(std::fstream& stream, const T* data, size_t size) {
	stream.write(reinterpret_cast<const char*>(data), size);
}

void Write(std::fstream& stream, void *data, size_t size);

template <typename T>
void Read(std::fstream& stream, T *data) {
	stream.read(reinterpret_cast<char*>(data), sizeof(T));
}

template <typename T>
void Read(std::fstream& stream, T& data) {
	stream.read(reinterpret_cast<char*>(&data), sizeof(T));
}

template <typename T>
void Read(std::fstream& stream, T *data, size_t size) {
	stream.read(reinterpret_cast<char*>(data), size);
}

template <typename T>
void Read(std::fstream& stream, T& data, size_t size) {
	stream.read(reinterpret_cast<char*>(&data), size);
}

void Read(std::fstream& stream, void *data, size_t size);