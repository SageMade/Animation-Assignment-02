#pragma once

#include "ParticleLayer.h"
#include "ParticleLayerSettings.h"
#include "ParticleEffectSettings.h"

#include <vector>
#include <cstdint>
#include <fstream>

class ParticleEffect {
public:
	std::vector<ParticleLayer*> Layers;
	char                        Name[EFFECT_NAME_MAX_LENGTH];

	ParticleEffect();
	~ParticleEffect();

	void Init();

	void AddLayer(ParticleLayerSettings settings);

	void Restart();

	void Update(float dt);
	void Draw();

	void WriteToFile(std::fstream& stream);
	static ParticleEffect ReadFromFile(std::fstream& stream);

	void BakeFboToBmp(const char * filename);
	void ResizeFbo(uint32_t width, uint32_t height);

	void ReplaceSettings(const ParticleEffectSettings& settings);

	glm::vec3 Origin;

private:
	uint32_t         myFramebuffer;
	uint32_t         myDepthRenderBuffer;
	uint32_t         myRenderTexture;
	uint32_t         myFboWidth, myFboHeight;
};