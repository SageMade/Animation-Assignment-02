

#pragma once

#include "GLM.h"

#include "ParticleLayerSettings.h"

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
	float mass;

	// Visual Properties
	float size;
	glm::vec4 colour;
	// Other properties... 
	// ie. sprite sheet

	float life;
	uint8_t texture;
};

// Emitter is responsible for emitting (creating, dispatching) particles
class ParticleLayer
{
public:
private:
	unsigned int m_pParticleArraySize;
	unsigned int m_pNumParticles;
	Particle* m_pParticles;
	float     m_pTimeSinceStart;
	float     m_pTimeSinceEmitted;

public:
	ParticleLayer(ParticleLayerSettings settings);
	ParticleLayer();
	~ParticleLayer();

	void initialize();
	void freeMemory();

	void restart() {
		m_pTimeSinceStart = 0;
		m_pTimeSinceEmitted = 0;
	}

	void update(float dt, glm::vec3 origin);
	void draw(glm::vec3 origin);

	void applyForceToParticle(unsigned int idx, glm::vec3 force);

	unsigned int getNumParticles() { return m_pNumParticles; }
	void setNumParticles(unsigned int numParticles) { m_pNumParticles = numParticles > m_pParticleArraySize ? m_pParticleArraySize : numParticles; }

	glm::vec3 getParticlePosition(unsigned int idx);
		
	ParticleLayerSettings Settings;
	
	// ... other properties
	// ... what would be a better way of doing this?
	// Make a keyframe controller for each property! this gives you max control!!
	// See the KeyframeController class
	// (this is what full out particle editors do, ex popcorn fx)
};