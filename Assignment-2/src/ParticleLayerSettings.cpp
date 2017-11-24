/*
	Authors:
	Shawn M.          100412327
	Paul Puig         100656910
	Stephen Richards  100458273
*/

#include "ParticleLayerSettings.h"

#include "FileHelpers.h"

#include "ParticleLayer.h"

void ParticleLayerSettings::WriteToFile(std::fstream & stream) {
	Write(stream, Config);
	Write(stream, (uint32_t)Behaviours.size());
	for (int ix = 0; ix < Behaviours.size(); ix++) {
		Behaviours[ix].WriteToFile(stream); 
	}
}

ParticleLayerSettings ParticleLayerSettings::ReadFromFile(std::fstream & stream)
{
	ParticleLayerSettings result = ParticleLayerSettings();
	Read(stream, result.Config);
	uint32_t size = 0;
	Read(stream, size);
	for (int ix = 0; ix < size; ix++) {
		SteeringBehaviour behaviour = SteeringBehaviour::ReadFromFile(stream);
		result.Behaviours.push_back(behaviour);
	}
	return result;
}
