#pragma once

#include <cstdint>
#include <glm\glm.hpp>

// Params for each particle
// Note: this is a bare minimum particle structure
// Add any properties you want to control here
struct Particle
{
	// Physics properties
	// Position is where the particle currently is
	// Velocity is the rate of change of position
	// Acceleration is the rate of change of velocity
	// p' = p + v*dt
	// current position is the previous position plus the change in position multiplied by the amount of time passed since we last calculated position
	// v' = v + a*dt
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 force;
	float angularVelocity;
	float mass;

	// Visual Properties
	float size;
	glm::vec4 colour;
	// Other properties... 
	// ie. sprite sheet
	float angle;
	float life;
	uint8_t texture;
};