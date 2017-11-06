#pragma once

#include <GLM/vec3.hpp>
#include <GLM/gtc/matrix_access.hpp>
#include <GLM/gtx/string_cast.hpp>
#include <iostream>

namespace Math
{
	// Linear interpolation
	template <typename T>
	T lerp(T d0, T d1, float t)
	{
		return (1 - t) * d0 + d1 * t;
	}

	float lerpRange(glm::vec2 range, float t);

	// inverse lerp
	// Regular lerp: given p0, p1 and a t value you get a point p between p0 and p1
	// Inverse lerp: given p0, p1 and p between p0 and p1 you will get the t value to compute p
	template <typename T>
	float invLerp(T d, T d0, T d1)
	{
		return (d - d0) / (d1 - d0);
	}
}