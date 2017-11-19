#include "SteeringBehaviour.h"
#include "FileHelpers.h"
#include "AnimationMath.h"
#include "EditorSettings.h"

#include <TTK\GraphicsUtils.h>

#include <iostream>

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
			{
				const PathData& path = *GetData<PathData>();
				if (path.Points.size() >= 2) {
					uint16_t currentNode = particle->pathData & MASK_PATH_NODEID;
					uint16_t nextNode = currentNode + (particle->pathData & MASK_PATH_REVERSE ? -1 : 1);

					if (currentNode >= path.Points.size())
						currentNode = path.Points.size() - 1;

					if (nextNode >= path.Points.size()) {
						switch (path.LoopMode) {
						case LoopType::LoopTypeLoop:
							nextNode = 0;
							break;
						case LoopType::LoopTypeStop:
							nextNode = currentNode;
							break;
						case LoopType::LoopTypeReverse:
							nextNode = currentNode - 1;
							particle->pathData |= MASK_PATH_REVERSE;
							break;
						}
					}
					glm::vec3 p0 = path.Points[currentNode];
					glm::vec3 p1 = path.Points[nextNode];
					if (Math::distSquared(p1, particle->position) < path.NodeRadius) {
						particle->pathData = nextNode | (particle->pathData & MASK_PATH_REVERSE);
					}
					else {
						glm::vec3 pDisp = p1 - p0;
						float segLen = pDisp.length();
						pDisp = glm::normalize(pDisp);
						glm::vec3 vRel  = particle->position - p0;
						float dot = glm::clamp(glm::dot(vRel, pDisp), 0.0f, 2 * segLen) + 0.2f;
						glm::vec3 pTarg = pDisp * dot;
						result = (pTarg - vRel);
						result *= (segLen - dot) / segLen;
						if (EditorSettings::DebugPaths) {
 							TTK::Graphics::DrawLine(p0, p0 + pTarg, 1.0f, RED);
							TTK::Graphics::DrawLine(p0, p1, 1.0f, GREEN);
							TTK::Graphics::DrawLine(particle->position, particle->position + result, 1.0f, BLUE);
						}
					}
				}
			}
			break;
	}
	return result;
}
