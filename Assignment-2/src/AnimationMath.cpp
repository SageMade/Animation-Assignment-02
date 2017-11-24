/*
	Authors:
	Shawn M.          100412327
	Paul Puig         100656910
	Stephen Richards  100458273
*/

#include "AnimationMath.h"

float Math::distSquared(glm::vec2 a, glm::vec2 b) {
	glm::vec2 temp = a - b;
	return temp.x * temp.x + temp.y * temp.y;
}

float Math::distSquared(glm::vec3 a, glm::vec3 b) {
	glm::vec3 temp = a - b;
	return temp.x * temp.x + temp.y * temp.y + temp.z * temp.z;
}

float Math::distSquared(glm::vec4 a, glm::vec4 b) {
	glm::vec4 temp = a - b;
	return temp.x * temp.x + temp.y * temp.y + temp.z * temp.z + temp.w * temp.w;
}

float Math::lerpRange(glm::vec2 range, float t) {
	return Math::lerp(range.x, range.y, t);
}