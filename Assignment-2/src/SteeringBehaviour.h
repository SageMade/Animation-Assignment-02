#pragma once

#include <cstdint>
#include <memory>
#include <fstream>
#include <glm\glm.hpp>
#include <vector>

#include "Particle.h"

#define BEHAVIOUR_NAME_SIZE 16

enum SteeringMethod : uint32_t {
	Unknown = 0,
	Seek = 1,
	Flee = 2,
	Repel = 3,
	Attract = 4,
	Path = 5
};

enum LoopType : uint32_t {
	LoopTypeLoop = 0,
	LoopTypeReverse = 1,
	LoopTypeStop = 2
};

struct SeekFleeData {
	glm::vec3 Point;
	float     Force;
};

struct MagnetData {
	glm::vec3 Point;
	float     Force;
	float     Radius;

	MagnetData() : Point(glm::vec3(0.0f)), Force(1.0f), Radius(1.0f) {}
};

struct PathData {
	std::vector<glm::vec3> Points;
	LoopType               LoopMode;
};

struct SteeringBehaviour {
	SteeringMethod Method;
	float          Weight;
	uint32_t       MetaSize;
	void*          MetaData;
	char           Name[BEHAVIOUR_NAME_SIZE];

	SteeringBehaviour() :
		Method(SteeringMethod::Unknown),
		Weight(1.0f),
		MetaSize(0),
		MetaData(nullptr) {
		memset(Name, '\0', BEHAVIOUR_NAME_SIZE);
	}

	void WriteToFile(std::fstream& stream);
	static SteeringBehaviour ReadFromFile(std::fstream& stream);

	template <typename T>
	void SetData(T *data) {
		MetaSize = sizeof(T);
		MetaData = data;
	}

	template<typename T>
	T* GetData() {
		return reinterpret_cast<T*>(MetaData);
	}

	glm::vec3 Apply(Particle* particle, float& totalWeight);
};