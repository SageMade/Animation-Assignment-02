

#include <iostream> // for std::cout
#include <glm\gtc\random.hpp> // for glm::linearRand
#include <TTK\GraphicsUtils.h> // for drawing utilities

#include "AnimationMath.h"
#include "ParticleLayer.h"
#include "TextureCollection.h"
#include "Renderer.h"
#include "ParticleEffect.h"

#define RAND_T   glm::linearRand(0.0f, 1.0f)
#define RAND_RNG glm::linearRand(-1.0f, 1.0f)

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

ParticleLayer::ParticleLayer(ParticleEffect* parent, ParticleLayerSettings settings) :
	m_pParticles(nullptr), 
	m_pNumParticles(0),
	m_pTimeSinceStart(0),
	Settings(settings),
	m_pTimeSinceEmitted(0),
	m_pEffect(parent) {

}

ParticleLayer::ParticleLayer(ParticleEffect* parent)
	: m_pParticles(nullptr),
	m_pNumParticles(0),
	m_pTimeSinceStart(0),
	m_pTimeSinceEmitted(0),
	m_pEffect(parent)
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
					particle->texture = Settings.Config.TextureID;
					particle->pathData = 0;

					float randomTval = glm::linearRand(0.0f, 1.0f);
					particle->colour = Math::lerp(Settings.Config.InitColor, Settings.Config.FinalColor, randomTval);

					float sizeMass = RAND_T;
					particle->size = Math::lerpRange(Settings.Config.SizeRange, sizeMass);
					particle->mass = Math::lerpRange(Settings.Config.MassRange, sizeMass);
					particle->life = Math::lerpRange(Settings.Config.LifeRange, sizeMass);

					switch (Settings.Config.BoundsType) {
						case EmitterType::Point:
							particle->position = Settings.Config.Position + origin;							
							break;
						case EmitterType::Box:
							particle->position = Settings.Config.Position + origin;
							particle->position.x += glm::linearRand(-Settings.Config.BoundsMeta.x, Settings.Config.BoundsMeta.x);
							particle->position.y += glm::linearRand(-Settings.Config.BoundsMeta.y, Settings.Config.BoundsMeta.y);
							particle->position.z += glm::linearRand(-Settings.Config.BoundsMeta.z, Settings.Config.BoundsMeta.z);
							break;
						case EmitterType::Circle:
							{
								particle->position = Settings.Config.Position + origin;
								glm::vec3 norm = glm::normalize(glm::vec3(RAND_RNG, RAND_RNG, RAND_RNG));
								particle->position += norm * Settings.Config.BoundsMeta;
							}
							break;
						case EmitterType::Line:
							particle->position = Settings.Config.Position + origin;
							particle->position += RAND_RNG * Settings.Config.BoundsMeta;
							break;
					}

					particle->angularVelocity = Math::lerpRange(Settings.Config.AngularSpeedRange, RAND_T);
					
					glm::vec3 vPos = glm::normalize(glm::vec3(RAND_RNG, RAND_RNG, RAND_RNG));
					vPos *= Settings.Config.VelocityRadius;
					vPos += Settings.Config.VelocityOffset;
					vPos = glm::normalize(vPos);
					particle->velocity = vPos * Math::lerpRange(Settings.Config.VelocityRange, RAND_T);
					
					emitted++;
				}
			}
			// TODO: apply behaviours
			glm::vec3 desiredVelocity;
			float totalWeight = 0;
			for (int ix = 0; ix < Settings.Behaviours.size(); ix++) {
				desiredVelocity += Settings.Behaviours[ix].Apply(particle, totalWeight);
			}
			desiredVelocity += m_pEffect->ApplyBehaviours(particle, totalWeight);
			
			if (totalWeight > 0 && (desiredVelocity.x * desiredVelocity.x + desiredVelocity.y * desiredVelocity.y + desiredVelocity.z * desiredVelocity.z) > 0.0f) {
				desiredVelocity /= totalWeight;
				//desiredVelocity = glm::normalize(desiredVelocity);
			}

			particle->force = desiredVelocity * particle->mass;
			
			// Update physics
			particle->force += Settings.Config.Gravity;

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
