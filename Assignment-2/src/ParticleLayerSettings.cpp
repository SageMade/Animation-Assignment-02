#include "ParticleLayerSettings.h"

#include "FileHelpers.h"

void ParticleLayerSettings::WriteToFile(std::fstream & stream) {
	Write(stream, &Config);
	stream << Behaviours.size();
	for (int ix = 0; ix < Behaviours.size(); ix++) {
		SteeringBehaviour behaviour = Behaviours[ix];
		Write(stream, &behaviour.Method);
		Write(stream, &behaviour.Weight);
		Write(stream, &behaviour.Name, BEHAVIOUR_NAME_SIZE);
		Write(stream, &behaviour.MetaSize);
		Write(stream, behaviour.MetaData, behaviour.MetaSize);
	}
}

ParticleLayerSettings ParticleLayerSettings::ReadFromFile(std::fstream & stream)
{
	ParticleLayerSettings result = ParticleLayerSettings();
	Read(stream, &result.Config, sizeof(result.Config));
	uint32_t size = 0;
	stream >> size;
	for (int ix = 0; ix < size; ix++) {
		SteeringBehaviour behaviour = SteeringBehaviour();
		Read(stream, &behaviour.Method);
		Read(stream, &behaviour.Weight);
		Read(stream, &behaviour.Name, BEHAVIOUR_NAME_SIZE);
		Read(stream, &behaviour.MetaSize);
		behaviour.MetaData = malloc(behaviour.MetaSize);
		stream.read(reinterpret_cast<char*>(&behaviour.MetaData), behaviour.MetaSize);
	}
	return result;
}
