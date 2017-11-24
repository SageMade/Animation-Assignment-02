/*
	Authors:
	Shawn M.          100412327
	Paul Puig         100656910
	Stephen Richards  100458273
*/

#pragma once

#include <vector>
#include "ParticleLayerSettings.h"

#define EFFECT_NAME_MAX_LENGTH 32

struct ParticleEffectSettings {
	char                               Name[EFFECT_NAME_MAX_LENGTH];
	std::vector<ParticleLayerSettings> Layers;
};