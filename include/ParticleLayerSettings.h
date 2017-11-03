#pragma once

#include <vector>

#include "GLM/glm.hpp"

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

struct SteeringBehaviour {
	SteeringMethod Method;
	float          Weight;
	void*          MetaData;
};

struct LayerConfig {
	glm::vec3   Position;
	glm::vec3   Gravity;
	float       EmissionRate;
	float       Duration;
	glm::vec4   InitColor;
	glm::vec4   FinalColor;

	glm::vec3   MinVelocity;
	glm::vec3   MaxVelocity;

	// Stores the range for initial particle life values
	glm::vec2   LifeRange;
	// Stores the range for initial particle size values
	glm::vec2   SizeRange;
	// Stores the range for particle masses
	glm::vec2   MassRange;

	EmitterType BoundsType;
	glm::vec3   BoundsMeta;

	bool        InterpolateColor;
};

struct ParticleLayerSettings {
	LayerConfig Config;	
	std::vector<SteeringBehaviour> Behaviours;
};