// Shaun McKinnon - 100642799 //
#include <iostream> // for std::cout
#include <glm\gtc\random.hpp> // for glm::linearRand
#include <TTK\GraphicsUtils.h> // for drawing utilities

#include "AnimationMath.h"
#include "ParticleEmitter.h"


glm::vec3 CalculateRandomConeNormal(glm::vec3 coneDir, float coneAngle) {
	glm::vec3 result;

	result.x = glm::linearRand(-1.0f, 1.0f);
	result.y = glm::linearRand(-1.0f, 1.0f);
	result.z = glm::linearRand(-1.0f, 1.0f);

	if (coneAngle < 180.0f){
		// Project onto plane perpindicular to conedir
		glm::vec3 projectedVector = result - (coneDir*(glm::dot(coneDir, result)));
		glm::mat4 rot = glm::mat4_cast(glm::angleAxis(glm::radians(coneAngle), projectedVector));
		result = (glm::vec4(coneDir.x, coneDir.y, coneDir.z, 1.0f) * rot);
	}
	return glm::normalize(result);
}

ParticleEmitter::ParticleEmitter()
	: m_pParticles(nullptr),
	m_pNumParticles(0),
	playing(true)
{

}

ParticleEmitter::~ParticleEmitter()
{
	freeMemory();
}

void ParticleEmitter::initialize(unsigned int numParticles)
{
	freeMemory(); // destroy any existing particles

	m_pParticleArraySize = numParticles;

	if (numParticles > 0)
	{
		m_pParticles = new Particle[numParticles];
		memset(m_pParticles, 0, sizeof(Particle) * numParticles);
		m_pNumParticles = numParticles;
	}
}

void ParticleEmitter::freeMemory()
{
	if (m_pParticles) // if not a null pointer
	{
		delete[] m_pParticles;
		m_pParticles = nullptr;
		m_pNumParticles = 0;
	}
}

void ParticleEmitter::update(float dt)
{
	if (m_pParticles && playing) // make sure memory is initialized and system is playing
	{
		// loop through each particle
		Particle* particle = m_pParticles;
		for (unsigned int i = 0; i < m_pNumParticles; ++i, ++particle)
		{
			if (particle->life <= 0) // if particle has no life remaining
			{
				// Respawn particle
				// Note: we are not freeing memory, we are "Recycling" the particles
				particle->acceleration = glm::vec3(0.0f);
				float randomTval = glm::linearRand(0.0f, 1.0f);
				particle->colour = Math::lerp(Settings.Config.InitColor, Settings.Config.FinalColor, randomTval);
				particle->life = Math::lerpRange(Settings.Config.LifeRange, randomTval);
				particle->mass = Math::lerpRange(Settings.Config.MassRange, randomTval);

				particle->position = Settings.Config.Position;

				particle->size =  Math::lerpRange(Settings.Config.SizeRange, randomTval);
				particle->velocity = Math::lerp(Settings.Config.MinVelocity, Settings.Config.MaxVelocity, glm::linearRand(0.0f, 1.0f));
			}
			
			// Update physics

			// Update acceleration (basic Newtonian physics)
			particle->acceleration = particle->force / particle->mass;
			particle->velocity = particle->velocity + (particle->acceleration * dt);
			particle->position = particle->position + particle->velocity * dt;

			// We've applied the force, let's remove it so it does not get applied next frame
			particle->force = glm::vec3(0.0f);

			// Decrease particle life
			particle->life -= dt;

			// Update visual properties
			if (Settings.Config.InterpolateColor)
			{
				// calculate t value
				float tVal = Math::invLerp(particle->life, Settings.Config.LifeRange.x, Settings.Config.LifeRange.y);
				particle->colour = Math::lerp(Settings.Config.InitColor, Settings.Config.FinalColor, 1-tVal);
			}
		}
	}
}

void ParticleEmitter::draw()
{
	// Draw the emitter position
	// Note: not necessary
	TTK::Graphics::DrawTeapot(Settings.Config.Position, 50.0f, glm::vec4(1.0f));
	
	TTK::Graphics::EnableAlpha();
	
	Particle* p = m_pParticles;
	for (int i = 0; i < m_pNumParticles; ++i, ++p)
	{
		if (p->life >= 0.0f) // if particle is alive, draw it
		{
			//TTK::Graphics::DrawCube(p->position, p->size, p->colour);
			//TTK::Graphics::DrawTeapot(p->position, p->size, p->colour); // low fps alert!! use with low particle count
			TTK::Graphics::DrawPoint(p->position, p->size, p->colour);
			// You can draw anything!
			// ...but you should stick to points in this framework since it uses GL 1.0
		}
	}
}

void ParticleEmitter::applyForceToParticle(unsigned int idx, glm::vec3 force)
{
	if (idx >= m_pNumParticles)
	{
		std::cout << "ParticleEmitter::applyForceToParticle ERROR: idx " << idx << "out of range!" << std::endl;
		return;
	}

	m_pParticles[idx].force = force;
}

glm::vec3 ParticleEmitter::getParticlePosition(unsigned int idx)
{
	if (idx >= m_pNumParticles)
	{
		std::cout << "ParticleEmitter::getParticlePosition ERROR: idx " << idx << "out of range!" << std::endl;
		return glm::vec3();
	}

	return m_pParticles[idx].position;
}
