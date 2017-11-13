#include "ParticleLayerSettings.h"

#include "FileHelpers.h"

void ParticleLayerSettings::WriteToFile(std::fstream & stream) {
	Write(stream, Config);
	Write(stream, (uint32_t)Behaviours.size());
	for (int ix = 0; ix < Behaviours.size(); ix++) {
		SteeringBehaviour behaviour = Behaviours[ix];
		Write(stream, behaviour.Method);
		Write(stream, behaviour.Weight);
		Write(stream, behaviour.Name, BEHAVIOUR_NAME_SIZE);
		Write(stream, behaviour.MetaSize);
		Write(stream, behaviour.MetaData, behaviour.MetaSize);
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

void SteeringBehaviour::WriteToFile(std::fstream & stream) {
	Write(stream, Method);
	Write(stream, Weight);
	Write(stream, Name, BEHAVIOUR_NAME_SIZE);
	Write(stream, MetaSize);
	Write(stream, MetaData, MetaSize);
}

SteeringBehaviour SteeringBehaviour::ReadFromFile(std::fstream & stream)
{
	SteeringBehaviour result = SteeringBehaviour();

	Read(stream, result.Method);
	Read(stream, result.Weight);
	Read(stream, result.Name, BEHAVIOUR_NAME_SIZE);
	Read(stream, result.MetaSize);
	if (result.MetaSize != 0) {
		result.MetaData = malloc(result.MetaSize);
		stream.read(reinterpret_cast<char*>(&result.MetaData), result.MetaSize);
	}

	return result;
}
