#pragma once

#include <vector>
#include <fstream>
#include <glm\glm.hpp>

#include "TTK/Texture2D.h"
#include "SteeringBehaviour.h"

#define MAX_PARTICLES_PER_LAYER 500
#define MAX_LAYER_NAME_SIZE 16

enum EmitterType : uint32_t {
	Point    = 0,
	Box      = 1,
	Circle   = 2,
	Line     = 3,
};

enum ParticleBlend : uint32_t {
	BlendMultiply  = 0,
	BlendAdditive  = 1
};

struct LayerConfig {
	glm::vec3   Position;
	glm::vec3   Gravity;
	float       EmissionRate;
	float       Duration;
	glm::vec4   InitColor;
	glm::vec4   FinalColor;
	char        Name[MAX_LAYER_NAME_SIZE];

	glm::vec3           VelocityRadius;
	glm::vec3           VelocityOffset;
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
		EmissionRate(10.0f),
		Duration(0.0f),
		InitColor(glm::vec4(1.0f)),
		FinalColor(glm::vec4(0.0f)),
		VelocityRadius(glm::vec3(1.0f)),
		VelocityOffset(glm::vec3(0.0f)),
		VelocityRange(glm::vec2(0.0f, 10.0f)),
		LifeRange(glm::vec2(0.0f, 5.0f)),
		SizeRange(glm::vec2(1.0f, 2.0f)),
		MassRange(glm::vec2(1.0f, 2.0f)),
		BoundsType(EmitterType::Box),
		BoundsMeta(glm::vec3(1.0f)),
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
