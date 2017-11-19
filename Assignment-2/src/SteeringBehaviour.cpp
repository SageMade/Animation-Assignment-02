#include "SteeringBehaviour.h"
#include "FileHelpers.h"

void SteeringBehaviour::WriteToFile(std::fstream & stream) {
	Write(stream, Method);
	Write(stream, Weight);
	Write(stream, Name, BEHAVIOUR_NAME_SIZE);
	if (Method != SteeringMethod::Path) {
		Write(stream, MetaSize);
		Write(stream, MetaData, MetaSize);
	}
	else {
		PathData data = *GetData<PathData>();
		Write(stream, data.LoopMode);
		uint32_t count = data.Points.size();
		Write(stream, count);
		for (int ix = 0; ix < count; ix++) {
			Write(stream, data.Points[ix]);
		}
	}
}

SteeringBehaviour SteeringBehaviour::ReadFromFile(std::fstream & stream)
{
	SteeringBehaviour result = SteeringBehaviour();

	Read(stream, result.Method);
	Read(stream, result.Weight);
	Read(stream, result.Name, BEHAVIOUR_NAME_SIZE);
	if (result.Method != SteeringMethod::Path) {
		Read(stream, result.MetaSize);

		if (result.MetaSize != 0) {
			result.MetaData = malloc(result.MetaSize);
			Read(stream, result.MetaData, result.MetaSize);
		}
		else {
			switch (result.Method) {
			case SteeringMethod::Attract:
			case SteeringMethod::Repel:
				result.SetData(new MagnetData());
				break;
			case SteeringMethod::Seek:
			case SteeringMethod::Flee:
				result.SetData(new SeekFleeData());
				break;
			}
		}
	}
	else {
		uint32_t count = 0;
		PathData *data = new PathData();
		Read(stream, data->LoopMode);
		Read(stream, count);
		for (int ix = 0; ix < count; ix++) {
			glm::vec3 point = glm::vec3();
			Read(stream, point);
			data->Points.push_back(point);
		}
		result.MetaData = data;
	}

	return result;
}

glm::vec3 SteeringBehaviour::Apply(Particle * particle, float& totalWeight) {
	totalWeight += Weight;
	glm::vec3 result = glm::vec3(0.0f);
	switch (Method) {
		case SteeringMethod::Attract:
		case SteeringMethod::Repel:
			{
				const MagnetData& data = *reinterpret_cast<MagnetData*>(MetaData);
				glm::vec3 displacement = particle->position - data.Point;
				float dist = displacement.length();
				displacement = glm::normalize(displacement);
				if (dist < data.Radius)
					result =  displacement * glm::clamp(dist / data.Radius, 0.0f, data.Force) * (Method == SteeringMethod::Attract ? -1.0f : 1.0f);
				else
					result = glm::normalize(displacement) * glm::clamp(dist, 0.0f, data.Force) *(Method == SteeringMethod::Attract ? -1.0f : 1.0f);
			}
			break;
		case SteeringMethod::Seek:
		case SteeringMethod::Flee:
			{
				const SeekFleeData& data = *reinterpret_cast<SeekFleeData*>(MetaData);
				glm::vec3 displacement = particle->position - data.Point;
				float dist = displacement.length();
				result = displacement * data.Force * (Method == SteeringMethod::Seek ? -1.0f : 1.0f);
			}
			break;
		case SteeringMethod::Path:
			// Ugh...
			break;
	}
	return result;
}
