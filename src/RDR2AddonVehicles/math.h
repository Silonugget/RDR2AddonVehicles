#pragma once
#include <math.h>

namespace Math {
	static float round_to(float value, float precision = 0.001f)
	{
		return std::round(value / precision) * precision;
	}

	static Vector3 round_to(Vector3 value, float precision = 0.001f)
	{
		return Vector3(std::round(value.x / precision) * precision, std::round(value.y / precision) * precision, std::round(value.z / precision) * precision);
	}

	static Vector3 RotationToDirection(Vector3 vRotation)
	{
		float radiansZ = vRotation.z * 0.0174532924f;
		float radiansX = vRotation.x * 0.0174532924f;
		float num = std::abs((float)std::cos((double)radiansX));
		Vector3 dir;
		dir.x = (float)((double)((float)(-(float)std::sin((double)radiansZ))) * (double)num);
		dir.y = (float)((double)((float)std::cos((double)radiansZ)) * (double)num);
		dir.z = (float)std::sin((double)radiansX);
		return dir;
	}
}