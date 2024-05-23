#include "ql_hack.h"
#include <iostream>

int QLHack::GetClosestToCrosshair(std::vector<Vector3> angles, Vector3 viewAnglesVec) {
	if (angles.size() == 0) {
		return false;
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

	return shortestPathIndex;
}

std::vector<Vector3> QLHack::GetEnemiesPosition() const {
	const auto playerTeam = memory.Read<int>(cgamex86 + offset::dwLocalPlayer + offset::team);
	const auto enemyTeam = playerTeam == 2 ? 2 : 1;
	const auto enemyTeamNumOffset = enemyTeam == 2 ? offset::redPlayersNum : offset::bluePlayersNum;
	const auto enemyPlayersNum = memory.Read<int>(cgamex86 + enemyTeamNumOffset);
	const auto playerList = qagamex86 + offset::dwPlayerList;
	auto enemyPlayersRead = 0;
	std::vector<Vector3> enemiesPosition;
	while (true && enemyPlayersNum > 0) {
		const auto playerAddress = playerList + enemyPlayersRead * 0xBD8;
		const auto isEnemy = memory.Read<int>(playerAddress + offset::team) != playerTeam;


		const auto enemyPlayerPosX = memory.Read<float>(playerAddress + offset::entityX);
		const auto enemyPlayerPosY = memory.Read<float>(playerAddress + offset::entityY);
		const auto enemyPlayerPosZ = memory.Read<float>(playerAddress + offset::entityZ);

		enemiesPosition.push_back(Vector3{ enemyPlayerPosX, enemyPlayerPosY, enemyPlayerPosZ });
		if (enemiesPosition.size() == enemyPlayersNum || enemyPlayersRead > 32) {
			break;
		}
	}

	return enemiesPosition;
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
		const auto closestEnemyIndex = QLHack::GetClosestToCrosshair(angles, viewAnglesVec);
		if (closestEnemyIndex != -1) {
			auto& closestEnemyPosition = positions[closestEnemyIndex];
			const auto enemyScreenPosition = closestEnemyPosition.WorldToScreen(QLHack::GetRefDef());
			QLHack::MoveMouse(enemyScreenPosition.x, enemyScreenPosition.y);
		}
	}
}

void QLHack::MoveMouse(int dx, int dy) {
	// Get the current mouse position
	POINT currentPos;
	if (GetCursorPos(&currentPos)) {
		// Create an INPUT structure to hold the event information
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_MOVE; // Use relative movement
		input.mi.dx = (dx - currentPos.x) / smoothing; // Relative movement in the x direction
		input.mi.dy = (dy - currentPos.y) / smoothing; // Relative movement in the y direction

		// Send the event
		SendInput(1, &input, sizeof(INPUT));
	}
	else {
		std::cerr << "Failed to get the current cursor position." << std::endl;
	}
}