#pragma once

#include "ParticleEmitter.h"
#include "ParticleLayerSettings.h"

#include <vector>
#include <cstdint>
#include <fstream>

class ParticleEffect {
public:
	std::vector<ParticleEmitter*> Layers;

	ParticleEffect();
	~ParticleEffect();

	void Init();

	void AddLayer(ParticleLayerSettings settings);

	void Update(float dt);
	void Draw();

	void WriteToFile(std::fstream& stream);
	static ParticleEffect ReadFromFile(std::fstream& stream);

	glm::vec3 Origin;

private:
	uint32_t         myFramebuffer;
	uint32_t         myDepthRenderBuffer;
	uint32_t         myRenderTexture;
};