#pragma once
#include <numbers>
#include <cmath>
#include "ref_def.h"

//struct pRefDef
//{
//public:
//	__int32 x; //0x0000 
//	__int32 y; //0x0004 
//	__int32 width; //0x0008 
//	__int32 height; //0x000C 
//	float fovY; //0x0010
//	float fovX;
//	Vector3 viewOrigin; //0x0018 
//	Vector3 viewAxis[3]; //0x0024 
//	__int32 time; //0x0048 
//	__int32 rdflags; //0x004C 
//	char unk[8];
//	float yaw;
//	float pitch;
//};//Size=0x0050

struct Vector3
{
	// constructor
	Vector3(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f) noexcept :
		x(x), y(y), z(z) { }

	// operator overloads
	const Vector3 operator-(const Vector3& other) const noexcept
	{
		return Vector3{ x - other.x, y - other.y, z - other.z };
	}

	const Vector3 operator+(const Vector3& other) const noexcept
	{
		return Vector3{ x + other.x, y + other.y, z + other.z };
	}

	const Vector3 operator/(const float factor) const noexcept
	{
		return Vector3{ x / factor, y / factor, z / factor };
	}

	const Vector3 operator*(const float factor) const noexcept
	{
		return Vector3{ x * factor, y * factor, z * factor };
	}

	const float dot(const Vector3 other) const
	{
		return (x * other.x + y * other.y + z * other.z);
	}

	const float comparePath(Vector3& other) const {
		const auto vector = Vector3{ x - other.x, y - other.y, z - other.z };
		return abs(vector.x) + abs(vector.y);
	}

	Vector3 WorldToScreen(pRefDef pRefDef);

	// utils
	const Vector3 ToAngle() const noexcept
	{
		return Vector3{
			std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>),
			std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
			0.0f };
	}

	const bool IsZero() const noexcept
	{
		return x == 0.f && y == 0.f && z == 0.f;
	}

	// struct data
	float x, y, z;
};