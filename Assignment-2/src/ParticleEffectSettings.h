#pragma once

#include <vector>
#include "ParticleLayerSettings.h"

#define EFFECT_NAME_MAX_LENGTH 32

struct ParticleEffectSettings {
	char                               Name[EFFECT_NAME_MAX_LENGTH];
	std::vector<ParticleLayerSettings> Layers;
};