#include "AnimationMath.h"

float Math::lerpRange(glm::vec2 range, float t) {
	return Math::lerp(range.x, range.y, t);
}