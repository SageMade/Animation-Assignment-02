

#include <iostream> // for std::cout
#include <glm\gtc\random.hpp> // for glm::linearRand
#include <TTK\GraphicsUtils.h> // for drawing utilities

#include "AnimationMath.h"
#include "ParticleLayer.h"
#include "TextureCollection.h"
#include "Renderer.h"

#define RAND_T glm::linearRand(0.0f, 1.0f)

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

ParticleLayer::ParticleLayer(ParticleLayerSettings settings) :
	m_pParticles(nullptr), 
	m_pNumParticles(0),
	m_pTimeSinceStart(0),
	Settings(settings),
	m_pTimeSinceEmitted(0) {

}

ParticleLayer::ParticleLayer()
	: m_pParticles(nullptr),
	m_pNumParticles(0),
	m_pTimeSinceStart(0),
	m_pTimeSinceEmitted(0)
{

}

ParticleLayer::~ParticleLayer()
{
	freeMemory();
}

void ParticleLayer::initialize()
{
	freeMemory(); // destroy any existing particles

	m_pParticleArraySize = Settings.Config.MaxParticles;

	if (m_pParticleArraySize > 0)
	{
		m_pParticles = new Particle[m_pParticleArraySize];
		memset(m_pParticles, 0, sizeof(Particle) * m_pParticleArraySize);
		m_pNumParticles = m_pParticleArraySize;
	}
}

void ParticleLayer::freeMemory()
{
	if (m_pParticles) // if not a null pointer
	{
		delete[] m_pParticles;
		m_pParticles = nullptr;
		m_pNumParticles = 0;
	}
}

void ParticleLayer::update(float dt, glm::vec3 origin)
{
	if (m_pParticles) // make sure memory is initialized and system is playing
	{
		m_pTimeSinceStart += dt;
		m_pTimeSinceEmitted += dt;

		bool spawnParticles = true;

		if (Settings.Config.Duration > 0 && m_pTimeSinceStart > Settings.Config.Duration)
			spawnParticles = false;

		int emitted = 0;
		// loop through each particle
		Particle* particle = m_pParticles;
		for (unsigned int i = 0; i < m_pNumParticles; ++i, ++particle)
		{
			if (particle->life <= 0 && spawnParticles) // if particle has no life remaining
			{
				// Caluclate particles per frame.
				// an emission rate of 1 should lead to 1 particle per second, or a spawn rate of 1 / 1s
				// an emmission rate of 120 should lead to 2 particles every frame, or a spawn rate of 120 / 1s
				// so the number of particles per frame is emissionRate * dt
				if (m_pTimeSinceEmitted > 1.0f / Settings.Config.EmissionRate && emitted < Settings.Config.EmissionRate * dt) {
					// Respawn particle
					// Note: we are not freeing memory, we are "Recycling" the particles
					particle->acceleration = glm::vec3(0.0f);
					float randomTval = glm::linearRand(0.0f, 1.0f);
					particle->colour = Math::lerp(Settings.Config.InitColor, Settings.Config.FinalColor, randomTval);
					particle->life = Math::lerpRange(Settings.Config.LifeRange, randomTval);
					particle->mass = Math::lerpRange(Settings.Config.MassRange, randomTval);

					particle->position = Settings.Config.Position + origin;

					particle->size = Math::lerpRange(Settings.Config.SizeRange, RAND_T);

					particle->angularVelocity = Math::lerpRange(Settings.Config.AngularSpeedRange, RAND_T);

					switch (Settings.Config.VelocityType) {
						default:
						case EmitterVelocityType::VelocityConeLine:
							particle->velocity = (Math::lerp(Settings.Config.Velocity0, Settings.Config.Velocity1, glm::linearRand(0.0f, 1.0f)));
							//particle->velocity *= glm::linearRand(Settings.Config.VelocityRange.x, Settings.Config.VelocityRange.y);

							break;
						case EmitterVelocityType::VelocityBox:
							particle->velocity = Math::lerp(Settings.Config.Velocity0, Settings.Config.Velocity1, glm::linearRand(0.0f, 1.0f));
							break;
					}

					emitted++;
				}
			}
			
			// Update physics
			particle->force += Settings.Config.Gravity;
			particle->texture = Settings.Config.TextureID;

			// Update acceleration (basic Newtonian physics)
			particle->acceleration = particle->force / particle->mass;
			particle->velocity = particle->velocity + (particle->acceleration * dt);
			particle->position = particle->position + particle->velocity * dt;

			// We've applied the force, let's remove it so it does not get applied next frame
			particle->force = glm::vec3(0.0f);

			// Decrease particle life
			particle->life -= dt;

			particle->angle += particle->angularVelocity * dt;

			// Update visual properties
			if (Settings.Config.InterpolateColor)
			{
				// calculate t value
				float tVal = Math::invLerp(particle->life, Settings.Config.LifeRange.x, Settings.Config.LifeRange.y);
				particle->colour = Math::lerp(Settings.Config.InitColor, Settings.Config.FinalColor, 1-tVal);
			}
		}

		if (emitted > 0)
			m_pTimeSinceEmitted = 0;
	}
}

void ParticleLayer::draw(glm::vec3 origin)
{	
	glEnable(GL_BLEND);

	switch (Settings.Config.BlendMode) {
		default:
		case ParticleBlend::BlendMultiply:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case ParticleBlend::BlendAdditive:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
	}

	Particle* p = m_pParticles;
	for (int i = 0; i < m_pNumParticles; ++i, ++p)
	{
		if (p->life >= 0.0f) // if particle is alive, draw it
		{
			Renderer::Submit(p->position, p->colour, p->angle, p->size, p->texture);
		}
	}

	Renderer::Flush();

}

void ParticleLayer::applyForceToParticle(unsigned int idx, glm::vec3 force)
{
	if (idx >= m_pNumParticles)
	{
		std::cout << "ParticleEmitter::applyForceToParticle ERROR: idx " << idx << "out of range!" << std::endl;
		return;
	}

	m_pParticles[idx].force = force;
}

glm::vec3 ParticleLayer::getParticlePosition(unsigned int idx)
{
	if (idx >= m_pNumParticles)
	{
		std::cout << "ParticleEmitter::getParticlePosition ERROR: idx " << idx << "out of range!" << std::endl;
		return glm::vec3();
	}

	return m_pParticles[idx].position;
}
