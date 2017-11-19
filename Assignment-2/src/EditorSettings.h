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

						

						
						