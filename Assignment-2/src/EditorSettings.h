/*
	Authors:
	Shawn M.          100412327
	Paul Puig         100656910
	Stephen Richards  100458273
*/

#pragma once

#include "ParticleEffectSettings.h"
#include "ParticleLayerSettings.h"

struct EditorSettings {
	static bool DebugPaths;

	static ParticleLayerSettings  *ActiveEditLayer;
	static ParticleEffectSettings  EffectSettings;

	static bool                    IsPlaying;
	static float                   PlaybackSpeed;
};

						

						
						