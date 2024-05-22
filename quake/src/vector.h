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

	//auto worldToScreen(ViewMatrix viewMatrix) -> Vector3
	//{
	//	const auto pos = Vector3{ x, y, z };
	//	const auto transform = Vector3{ viewMatrix._13, viewMatrix._23, viewMatrix._33 };
	//	Vector3 right = { viewMatrix._11, viewMatrix._12, viewMatrix._13 };
	//	Vector3 up = { viewMatrix._21, viewMatrix._22, viewMatrix._23 };

	//	float w = transform.dot(pos) + viewMatrix._34;

	//	if (w < 0.099f)
	//		return false;

	//	float x = right.dot(pos) + viewMatrix._14;
	//	float y = up.dot(pos) + viewMatrix._24;

	//	return Vector3{ (1920 / 2) * (1.f + x / w), (1080 / 2) * (1.f - y / w) };
	//}

	//Vector3 WorldToScreen(ViewMatrix viewMatrix, int windowWidth, int windowHeight)
	//{
	//	const auto pos = Vector3{ x, y, z };
	//	const auto matrixX = pos.x * viewMatrix._11 + pos.y * viewMatrix._12 + pos.z * viewMatrix._13 + viewMatrix._14;
	//	const auto matrixY = pos.x * viewMatrix._21 + pos.y * viewMatrix._22 + pos.z * viewMatrix._23 + viewMatrix._24;
	//	const auto matrixZ = pos.x * viewMatrix._31 + pos.y * viewMatrix._32 + pos.z * viewMatrix._33 + viewMatrix._34;
	//	const auto matrixW = pos.x * viewMatrix._41 + pos.y * viewMatrix._42 + pos.z * viewMatrix._43 + viewMatrix._44;

	//	//if (matrixW < 0.1f)
	//	//	return false;

	//	//perspective division, dividing by clip.W = Normalized Device Coordinates
	//	Vector3 NDC;
	//	NDC.x = matrixX / matrixW;
	//	NDC.y = matrixY / matrixW;
	//	NDC.z = matrixZ / matrixW;

	//	const auto screenX = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	//	const auto screenY = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
	//	return Vector3{ screenX, screenY };
	//}

	Vector3 WorldToScreen(pRefDef& pRefDef)
	{
		const auto viewOriginVec = Vector3{ pRefDef.viewOrigin[0], pRefDef.viewOrigin[1],pRefDef.viewOrigin[2] };
		const auto viewAxisX = Vector3{ pRefDef.viewAxis[0], pRefDef.viewAxis[1],pRefDef.viewAxis[2] };
		const auto viewAxisY = Vector3{ pRefDef.viewAxis[3], pRefDef.viewAxis[4],pRefDef.viewAxis[5] };
		const auto viewAxisZ = Vector3{ pRefDef.viewAxis[6], pRefDef.viewAxis[7],pRefDef.viewAxis[8] };
		const auto pos = Vector3{ x, y, z };
		auto screen = Vector3{};
		Vector3 trans;
		float xc, yc;
		float px, py;
		float z;

		px = tan(pRefDef.fovY * 3.14 / 360.0);
		py = tan(pRefDef.fovX * 3.14 / 360.0);

		trans = pos - viewOriginVec;

		xc = 1920 / 2.0;
		yc = 1080 / 2.0;

		z = trans.dot(viewAxisX);
		if (z <= 0.001)
			return false;

		screen.x = xc - trans.dot(viewAxisY) * xc / (z * px);
		screen.y = yc - trans.dot(viewAxisZ) * yc / (z * py);
		return screen;
	}

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