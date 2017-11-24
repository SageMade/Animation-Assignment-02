/*
	Authors:
		Shawn M.          100412327
		Paul Puig         100656910
		Stephen Richards  100458273
*/

#include "EditorSettings.h"

bool EditorSettings::DebugPaths = false;

ParticleLayerSettings  *EditorSettings::ActiveEditLayer = nullptr;
ParticleEffectSettings  EditorSettings::EffectSettings;

bool                    EditorSettings::IsPlaying = true;
float                   EditorSettings::PlaybackSpeed = 100.0f;