#pragma once
#include <cstdint>
#include <vector>
#include <vector>
#include <vector>
#include "vector.h"
#include "memory.h"
#include "offset.h"
#include "ref_def.h"
#include "timer.h"
#include "entity.h"

class QLHack
{
private:


public:
	Memory memory;
	uintptr_t cgamex86;
	uintptr_t qagamex86;
	uintptr_t quake;
	bool wallhack;
	bool aimbot;
	Timer railGunTimer;
	float smoothing;
	float fov;

	QLHack(bool wallhack, bool aimbot, float smoothing, float fov) noexcept
		: memory("quakelive_steam.exe"),
		cgamex86(memory.GetModuleAddress("cgamex86.dll")),
		qagamex86(memory.GetModuleAddress("qagamex86.dll")),
		quake(memory.GetModuleAddress("quakelive_steam.exe")),
		wallhack(wallhack),
		aimbot(aimbot),
		railGunTimer(Timer{}),
		smoothing(smoothing),
		fov(fov)
	{
	}

	Vector3 CalculateAngle(
		const Vector3& localPosition,
		const Vector3& enemyPosition,
		const Vector3& viewAngles) noexcept
	{
		const auto distanceAngle = (enemyPosition - localPosition).ToAngle();
		return distanceAngle - viewAngles;
	}

	pRefDef GetRefDef() const {
		return memory.Read<pRefDef>(cgamex86 + offset::dwRefDef);
	}

	std::pair<int, double> GetClosestToCrosshair(std::vector<Vector3> angles, Vector3 viewAnglesVec) const;

	std::vector<Vector3> GetEnemiesPosition() const;

	void GetEnemyPlayers();

	void MoveMouse(int dx, int dy) const;

	void MouseClick() const;

	float ClampMouseRelativeMovement(float currentPos, float relativePos, float upperBounds) const;

	std::vector<Entity> GetEnemyEntities() const;
};