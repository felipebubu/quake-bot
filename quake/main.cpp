#include "src/memory.h"
#include "src/vector.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "src/timer.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <dwmapi.h>
#include <d3d11.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)) {
		return 0L;
	}

	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return 0L;
	}

	return DefWindowProc(window, message, wParam, lParam);
}

namespace offset
{
	//cgamex
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xA9C210;
	constexpr ::std::ptrdiff_t yaw = 0xA4;
	constexpr ::std::ptrdiff_t pitch = 0xA0;

	//quakelive.exe
	constexpr ::std::ptrdiff_t yawSum = 0x1072754;
	constexpr ::std::ptrdiff_t pitchSum = 0x1072758;

	//qagamex
	constexpr ::std::ptrdiff_t dwPlayerList = 0x5ADD58;
	constexpr ::std::ptrdiff_t dwPlayerLength = 0xBD8;

	//qagamex + entity
	constexpr ::std::ptrdiff_t entityX = 0x14;
	constexpr ::std::ptrdiff_t entityY = 0x18;
	constexpr ::std::ptrdiff_t entityZ = 0x1C;
	constexpr ::std::ptrdiff_t team = 0x10C;

	//cgamex
	constexpr ::std::ptrdiff_t redPlayersNum = 0xA404BC;
	constexpr ::std::ptrdiff_t bluePlayersNum = 0xA404C0;

}

void getEntityList(Memory memory, uintptr_t quake) {
	auto index = memory.Read<uintptr_t>(quake + 0x1316DE4);
	//const auto base = memory.Read<uintptr_t>(quake + 0x1345A78);
	//index = index + index * 0x2 + 0x2016;
	//index = index << 6;
	//const auto entity = memory.Read<uintptr_t>(quake);
}

Vector3 calculateAngle(
	const Vector3& localPosition,
	const Vector3& enemyPosition,
	const Vector3& viewAngles) noexcept
{
	const auto distanceAngle = (enemyPosition - localPosition).ToAngle();
	return distanceAngle - viewAngles;
}

Vector3 getClosestToCrosshair(std::vector<Vector3> angles, Vector3 viewAnglesVec) {
	if (angles.size() == 0) {
		return false;
	}

	Vector3 shortestPath = Vector3{ 0, 0 };
	int count = 0;
	int shortestDistance = 20;
	for (const Vector3 angle : angles) {
		Vector3 newAngle = viewAnglesVec + angle;
		auto distance = viewAnglesVec.comparePath(newAngle);
		if (distance < shortestDistance) {
			shortestDistance = distance;
			shortestPath = angle;
		}
		count++;
	}

	return shortestPath;
}

void getEnemyPlayers(Memory& memory, uintptr_t cgamex86, uintptr_t qagamex86, uintptr_t quake) {
	const auto playerTeam = memory.Read<int>(cgamex86 + offset::dwLocalPlayer + offset::team);
	const auto enemyTeam = playerTeam == 2 ? 2 : 1;
	const auto enemyTeamNumOffset = enemyTeam == 2 ? offset::redPlayersNum : offset::bluePlayersNum;
	const auto enemyPlayersNum = memory.Read<int>(cgamex86 + enemyTeamNumOffset);
	const auto playerList = qagamex86 + offset::dwPlayerList;
	auto enemyPlayersRead = 0;
	std::vector<Vector3> angles;
	while (true && enemyPlayersNum > 0) {
		const auto playerAddress = playerList + enemyPlayersRead * 0xBD8;
		const auto isEnemy = memory.Read<int>(playerAddress + offset::team) != playerTeam;

		const auto localPlayerPosX = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::entityX);
		const auto localPlayerPosY = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::entityY);
		const auto localPlayerPosZ = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::entityZ);

		const auto pitch = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::pitch);
		const auto yaw = memory.Read<float>(cgamex86 + offset::dwLocalPlayer + offset::yaw);

		const auto enemyPlayerPosX = memory.Read<float>(playerAddress + offset::entityX);
		const auto enemyPlayerPosY = memory.Read<float>(playerAddress + offset::entityY);
		const auto enemyPlayerPosZ = memory.Read<float>(playerAddress + offset::entityZ);

		auto playerVec = Vector3{ localPlayerPosX, localPlayerPosY, localPlayerPosZ };
		auto enemyPlayerVec = Vector3{ enemyPlayerPosX, enemyPlayerPosY, enemyPlayerPosZ };
		auto viewAnglesVec = Vector3{ yaw, pitch };
		Vector3 angleVec = calculateAngle(playerVec, enemyPlayerVec, viewAnglesVec);
		const auto fov = std::hypot(angleVec.x, angleVec.y);
		const auto newAngleVec = (viewAnglesVec + angleVec) - viewAnglesVec;

		if (isEnemy) {
			angles.push_back(newAngleVec);
		}
		if (angles.size() == enemyPlayersNum || enemyPlayersRead > 1) {
			if (angles.size() == 0) {
				break;
			}
			const auto closestEnemyVec = getClosestToCrosshair(angles, viewAnglesVec);
			auto pitchSum = memory.Read<float>(quake + offset::pitchSum);
			auto yawSum = memory.Read<float>(quake + offset::yawSum);
			yawSum += closestEnemyVec.y / 253.f;
			pitchSum += closestEnemyVec.x / 253.f;
			memory.Write<float>(quake + offset::pitchSum, pitchSum);
			memory.Write<float>(quake + offset::yawSum, yawSum);
			break;
		}
		enemyPlayersRead++;
	}
}


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
	WNDCLASSEXW wc{};

	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = windowProcedure;
	wc.hInstance = instance;
	wc.lpszClassName = L"cool name";

	RegisterClassExW(&wc);

	const HWND window = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		wc.lpszClassName,
		L"cool name",
		WS_POPUP,
		0,
		0,
		1920,
		1080,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
	{
		RECT clientArea{};
		GetClientRect(window, &clientArea);

		RECT windowArea{};
		GetWindowRect(window, &windowArea);

		POINT diff{};

		ClientToScreen(window, &diff);

		const MARGINS margins{
			windowArea.left + (diff.x - windowArea.left),
			windowArea.top + (diff.y - windowArea.top),
			clientArea.right,
			clientArea.bottom
		};

		DwmExtendFrameIntoClientArea(window, &margins);
	}

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2U;
	sd.OutputWindow = window;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	constexpr D3D_FEATURE_LEVEL levels[2]{

	}

	Memory memory = Memory{ "quakelive_steam.exe" };

	const auto cgamex86 = memory.GetModuleAddress("cgamex86.dll");
	const auto qagamex86 = memory.GetModuleAddress("qagamex86.dll");
	const auto quake = memory.GetModuleAddress("quakelive_steam.exe");

	auto railGunTimer = Timer{};

	while (true) {
		const auto pitch = memory.Read<float>(quake + offset::pitch);
		const auto yaw = memory.Read<float>(quake + offset::yaw);

		const auto localPlayerX = memory.Read<float>(cgamex86 + offset::entityX);
		const auto y = memory.Read<float>(cgamex86 + offset::entityY);

		//while (GetAsyncKeyState(VK_XBUTTON2)) {
		//	getEnemyPlayers(memory, cgamex86, qagamex86, quake);
		//	//std::this_thread::sleep_for(std::chrono::milliseconds(5));
		//}

		while (GetAsyncKeyState(VK_XBUTTON1) && railGunTimer.elapsed() > 1.4f) {
			auto railGunShootingTimer = Timer{};
			while (railGunShootingTimer.elapsed() < 0.05) {
				getEnemyPlayers(memory, cgamex86, qagamex86, quake);
			}
			railGunTimer.reset();
		}
	}
}
