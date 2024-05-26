#include "ql_hack.h"
#include <iostream>
#include <random>
#include "snap.h"

std::pair<int, double> QLHack::GetClosestToCrosshair(std::vector<Vector3> angles, Vector3 viewAnglesVec) const {
	if (angles.size() == 0) {
		return {};
	}

	int shortestPathIndex = -1;
	int count = 0;
	int shortestDistance = fov;
	for (const Vector3 angle : angles) {
		Vector3 newAngle = viewAnglesVec + angle;
		auto distance = viewAnglesVec.comparePath(newAngle);
		if (distance < shortestDistance) {
			shortestDistance = distance;
			shortestPathIndex = count;
		}
		count++;
	}

	return { shortestPathIndex, shortestDistance };
}

std::vector<Vector3> QLHack::GetEnemiesPosition() const {
	const auto playerTeam = memory.Read<int>(cgamex86 + offset::dwLocalPlayer + offset::team);
	const auto enemyTeam = playerTeam == 2 ? 2 : 1;
	const auto enemyTeamNumOffset = enemyTeam == 2 ? offset::redPlayersNum : offset::bluePlayersNum;
	const auto enemyPlayersNum = memory.Read<int>(cgamex86 + enemyTeamNumOffset);
	const auto playerList = qagamex86 + offset::dwPlayerList;
	auto enemyPlayersRead = 0;
	std::vector<Vector3> enemiesPosition;
	/*while (true && enemyPlayersNum > 0) {
		const auto playerAddress = playerList + enemyPlayersRead * 0xBD8;
		const auto isEnemy = memory.Read<int>(playerAddress + offset::team) != playerTeam;

		const auto enemyPlayerPosX = memory.Read<float>(playerAddress + offset::);
		const auto enemyPlayerPosY = memory.Read<float>(playerAddress + offset::entityY);
		const auto enemyPlayerPosZ = memory.Read<float>(playerAddress + offset::entityZ);

		const auto enemyPos = Vector3{ enemyPlayerPosX, enemyPlayerPosY, enemyPlayerPosZ };
		if (!(enemyPos.x == 0 && enemyPos.y == 0 && enemyPos.z == 0)) {
			enemiesPosition.push_back(Vector3{ enemyPlayerPosX, enemyPlayerPosY, enemyPlayerPosZ });
		}
		if (enemiesPosition.size() == enemyPlayersNum || enemyPlayersRead > 32) {
			break;
		}
		enemyPlayersRead++;
	}*/

	const auto entities = QLHack::GetEnemyEntities();
	auto positions = std::vector<Vector3>{};
	for (const Entity entity : entities) {
		positions.push_back(Vector3{ entity.posX, entity.posY, entity.posZ });
	}

	return positions;
}

void QLHack::GetEnemyPlayers() {
	auto positions = QLHack::GetEnemiesPosition();
	std::vector<Vector3> angles;

	const auto pitch = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::pitch);
	const auto yaw = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::yaw);
	auto viewAnglesVec = Vector3{ yaw, pitch };

	for (Vector3 position : positions) {
		const auto localPlayerPosX = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::entityX);
		const auto localPlayerPosY = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::entityY);
		const auto localPlayerPosZ = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::entityZ);

		auto playerVec = Vector3{ localPlayerPosX, localPlayerPosY, localPlayerPosZ };
		auto enemyPlayerVec = Vector3{ position.x, position.y, position.z };
		Vector3 angleVec = QLHack::CalculateAngle(playerVec, enemyPlayerVec, viewAnglesVec);
		const auto fov = std::hypot(angleVec.x, angleVec.y);
		const auto newAngleVec = (viewAnglesVec + angleVec) - viewAnglesVec;
		angles.push_back(newAngleVec);
	}

	if (angles.size() > 0) {
		const auto indexDistancePair = QLHack::GetClosestToCrosshair(angles, viewAnglesVec);
		if (indexDistancePair.first != -1) {
			auto& closestEnemyPosition = positions[indexDistancePair.first];
			const auto enemyScreenPosition = closestEnemyPosition.WorldToScreen(QLHack::GetRefDef());
			QLHack::MoveMouse(enemyScreenPosition.x, enemyScreenPosition.y);

			if (indexDistancePair.second < 4) {
				QLHack::MouseClick();
				QLHack::railGunTimer.reset();
			}
		}
	}
}

void QLHack::MoveMouse(int dx, int dy) const {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> deltaDistribution(0, 1); // distribution in range [1, 6]

	const float delta = deltaDistribution(rng);
	// Get the current mouse position
	POINT currentPos;
	if (GetCursorPos(&currentPos)) {
		// Create an INPUT structure to hold the event information
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_MOVE; // Use relative movement
		input.mi.dx = ((dx - currentPos.x) / smoothing) + (2.f - delta); // Relative movement in the x direction
		input.mi.dy = ((dy - currentPos.y) / smoothing) + (2.f - delta); // Relative movement in the y direction

		input.mi.dx = QLHack::ClampMouseRelativeMovement(currentPos.x, input.mi.dx, 1920);
		input.mi.dy = QLHack::ClampMouseRelativeMovement(currentPos.y, input.mi.dy, 1080);
		// Send the event
		SendInput(1, &input, sizeof(INPUT));
	}
	else {
		std::cerr << "Failed to get the current cursor position." << std::endl;
	}
}

void QLHack::MouseClick() const {
	// Create an array of INPUT structures to hold the event information
	INPUT inputs[2] = {};

	// Set up the first INPUT structure for the mouse down event
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	// Set up the second INPUT structure for the mouse up event
	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	// Send the events
	SendInput(2, inputs, sizeof(INPUT));
}

float QLHack::ClampMouseRelativeMovement(float currentPos, float relativePos, float upperBounds) const {
	if (currentPos + relativePos > upperBounds) {
		return (currentPos + relativePos) - upperBounds;
	}

	else if (currentPos + relativePos < 1) {
		return 1;
	}

	return relativePos;
}

std::vector<Entity> QLHack::GetEnemyEntities() const {
	//	const auto dwEntities = memory.Read<uintptr_t>(cgamex86 + offset::entities + 0x2D0);
	auto entities = std::vector<Entity>{};
	const auto dwSnap = memory.Read<uintptr_t>(cgamex86 + offset::snap);
	const auto numEntities = memory.Read<uintptr_t>(dwSnap + 0x27C);
	for (int i = 0; i < numEntities; i++) {
		const auto dwEntityIndex = memory.Read<uintptr_t>(dwSnap + (i * 0xEC) + 0x280);
		const auto dwEntity = cgamex86 + offset::entities + 0x2d0 * dwEntityIndex;
		const auto entity = memory.Read<Entity>(dwEntity);
		if (entity.type == 1) {
			entities.push_back(entity);
		}
	}

	return entities;
}