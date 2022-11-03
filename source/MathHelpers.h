#pragma once
#include <cmath>

namespace dae
{
	/* --- CONSTANTS --- */
	constexpr auto PI = 3.14159f;
	constexpr auto PI_DIV_2 = 1.57079f;
	constexpr auto PI_DIV_4 = 0.78539f;
	constexpr auto PI_2 = 6.28318f;
	constexpr auto PI_4 = 12.56637f;
	constexpr auto PI_INV = 0.31831f;

	constexpr auto TO_DEGREES = (180.f / PI);
	constexpr auto TO_RADIANS(PI / 180.f);

	inline float Square(float a)
	{
		return a * a;
	}

	inline float Lerpf(float a, float b, float factor)
	{
		return ((1 - factor) * a) + (factor * b);
	}

	inline bool AreEqual(float a, float b, float epsilon = FLT_EPSILON)
	{
		return abs(a - b) < epsilon;
	}
}