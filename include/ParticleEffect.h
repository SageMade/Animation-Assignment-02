#pragma once

#include "ParticleEmitter.h"
#include "ParticleLayerSettings.h"

#include <vector>
#include <cstdint>

class ParticleEffect {
public:
	ParticleEffect();
	~ParticleEffect();

	void AddLayer(ParticleLayerSettings settings);

	void Update(float dt);
	void Draw();

private:
	uint8_t          myLayerCount;
	ParticleEmitter *myLayers;
};