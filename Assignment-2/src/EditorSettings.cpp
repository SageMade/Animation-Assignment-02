#include "EditorSettings.h"

bool EditorSettings::DebugPaths = true;

ParticleLayerSettings  *EditorSettings::ActiveEditLayer = nullptr;
ParticleEffectSettings  EditorSettings::EffectSettings;

bool                    EditorSettings::IsPlaying = true;
float                   EditorSettings::PlaybackSpeed = 100.0f;