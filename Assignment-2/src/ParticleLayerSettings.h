#pragma once

#include <vector>

#include "TTK/Texture2D.h"

#include "GLM/glm.hpp"

#include <fstream>

#define MAX_PARTICLES_PER_LAYER 500
#define BEHAVIOUR_NAME_SIZE 16
#define MAX_LAYER_NAME_SIZE 16

enum SteeringMethod {
	Unknown  = 0,
	Seek     = 1,
	Flee     = 2,
	Repel    = 3,
	Attract  = 4,
	Path     = 5
};

enum EmitterType {
	Point    = 0,
	Box      = 1,
	Circle   = 2,
	Line     = 3,
};

enum EmitterVelocityType {
	VelocityConeLine = 0,
	VelocityBox      = 1
};

enum ParticleBlend {
	BlendMultiply  = 0,
	BlendAdditive  = 1
};

enum LoopType {
	LoopTypeLoop    = 0,
	LoopTypeReverse = 1,
	LoopTypeStop    = 2
};

struct SeekFleeData {
	glm::vec3 Point;
	bool      LocalSpace;
};

struct MagnetData {
	glm::vec3 Point;
	float     Force;
	bool      LocalSpace;
};

struct PathData {
	std::vector<glm::vec3> Points;
	bool                   LocalSpace;
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

	template <typename T>
	void SetData(T *data) {
		MetaSize = sizeof(T);
		MetaData = data;
	}

	template<typename T>
	T* GetData() {
		return reinterpret_cast<T*>(MetaData);
	}
};

struct LayerConfig {
	glm::vec3   Position;
	glm::vec3   Gravity;
	float       EmissionRate;
	float       Duration;
	glm::vec4   InitColor;
	glm::vec4   FinalColor;
	char        Name[MAX_LAYER_NAME_SIZE];

	EmitterVelocityType VelocityType;
	glm::vec3           Velocity0;
	glm::vec3           Velocity1;
	glm::vec2           VelocityRange;

	// Stores the range for initial particle life values
	glm::vec2     LifeRange;
	// Stores the range for initial particle size values
	glm::vec2     SizeRange;
	// Stores the range for particle masses
	glm::vec2     MassRange;

	EmitterType   BoundsType;
	glm::vec3     BoundsMeta;

	uint8_t       TextureID;

	uint8_t       Index;

	uint32_t      MaxParticles;

	ParticleBlend BlendMode;
	bool          InterpolateColor;

	// Stores the range for angular speed range
	glm::vec2     AngularSpeedRange;

	LayerConfig() :
		Position(glm::vec3(0.0f)),
		Gravity(glm::vec3(0.0f)),
		EmissionRate(100.0f),
		Duration(0.0f),
		InitColor(glm::vec4(1.0f)),
		FinalColor(glm::vec4(0.0f)),
		VelocityType(EmitterVelocityType::VelocityConeLine),
		Velocity0(glm::vec3(-10.0f, 10.0f, 0.0f)),
		Velocity1(glm::vec3( 10.0f, 10.0f, 0.0f)),
		VelocityRange(glm::vec2(0.0f)),
		LifeRange(glm::vec2(0.0f, 5.0f)),
		SizeRange(glm::vec2(10.0f, 25.0f)),
		MassRange(glm::vec2(1.0f)),
		BoundsType(EmitterType::Point),
		BoundsMeta(glm::vec3(0.0f)),
		BlendMode(ParticleBlend::BlendAdditive),
		InterpolateColor(true),
		MaxParticles(MAX_PARTICLES_PER_LAYER),
		TextureID(0),
		Index(0),
		AngularSpeedRange(glm::vec2(-1.0f, 1.0f))
		{
	}
};

struct ParticleLayerSettings {
	LayerConfig Config;	
	std::vector<SteeringBehaviour> Behaviours;

	void WriteToFile(std::fstream& stream);
	static ParticleLayerSettings ReadFromFile(std::fstream& stream);
};
