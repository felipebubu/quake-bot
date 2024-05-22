#include "memory.h"
#include "vector.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "timer.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <dwmapi.h>
#include <d3d11.h>
#include "ref_def.h"
#include "vector.h"
#include "view_matrix.h"
#include <thread>

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
	constexpr ::std::ptrdiff_t dwRefDef = 0xA9C820;

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
	//constexpr ::std::ptrdiff_t viewMatrix = 0x;

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
	int shortestDistance = 10;
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
std::vector<Vector3> getEnemiesPosition(Memory& memory, uintptr_t cgamex86, uintptr_t qagamex86, uintptr_t quake) {
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

void getEnemyPlayers(Memory& memory, uintptr_t cgamex86, uintptr_t qagamex86, uintptr_t quake, double smoothing) {
	const auto positions = getEnemiesPosition(memory, cgamex86, qagamex86, quake);
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
		Vector3 angleVec = calculateAngle(playerVec, enemyPlayerVec, viewAnglesVec);
		const auto fov = std::hypot(angleVec.x, angleVec.y);
		const auto newAngleVec = (viewAnglesVec + angleVec) - viewAnglesVec;
		angles.push_back(newAngleVec);
	}

	if (angles.size() > 0) {
		const auto closestEnemyVec = getClosestToCrosshair(angles, viewAnglesVec);
		auto pitchSum = memory.Read<float>(quake + offset::pitchSum);
		auto yawSum = memory.Read<float>(quake + offset::yawSum);
		yawSum += closestEnemyVec.y / smoothing;
		pitchSum += closestEnemyVec.x / smoothing;
		memory.Write<float>(quake + offset::pitchSum, pitchSum);
		memory.Write<float>(quake + offset::yawSum, yawSum);
	}
}


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmdShow) {
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
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
	};

	ID3D11Device* device{ nullptr };
	ID3D11DeviceContext* deviceContext{ nullptr };
	IDXGISwapChain* swapChain{ nullptr };
	ID3D11RenderTargetView* renderTargetView{ nullptr };
	D3D_FEATURE_LEVEL level{};

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		levels,
		2U,
		D3D11_SDK_VERSION,
		&sd,
		&swapChain,
		&device,
		&level,
		&deviceContext);

	ID3D11Texture2D* backBuffer{ nullptr };
	swapChain->GetBuffer(0U, IID_PPV_ARGS(&backBuffer));

	if (backBuffer) {
		device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
		backBuffer->Release();
	}
	else {
		return 1;
	}

	ShowWindow(window, cmdShow);
	UpdateWindow(window);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, deviceContext);

	Memory memory = Memory{ "quakelive_steam.exe" };

	const auto cgamex86 = memory.GetModuleAddress("cgamex86.dll");
	const auto qagamex86 = memory.GetModuleAddress("qagamex86.dll");
	const auto quake = memory.GetModuleAddress("quakelive_steam.exe");
	auto railGunTimer = Timer{};

	bool running = true;

	while (running) {
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				running = false;
			}
		}

		if (!running) {
			break;
		}


		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();


		//const auto viewMatrix = memory.Read<ViewMatrix>(0x1E67514);
		auto refDef = memory.Read<pRefDef>(cgamex86 + offset::dwRefDef);
		const auto positions = getEnemiesPosition(memory, cgamex86, qagamex86, quake);
		for (Vector3 position : positions) {
			const auto screenCoords = position.WorldToScreen(refDef);
			ImGui::GetBackgroundDrawList()->AddRect({ screenCoords.x - 10.f, screenCoords.y - 10.f },
				{ screenCoords.x + 20.f, screenCoords.y + 50.f }, ImColor(1.f, 0.f, 0.f));
		}
		ImGui::Render();

		constexpr float color[4]{ 0.f, 0.f, 0.f, 0.f };
		deviceContext->OMSetRenderTargets(1U, &renderTargetView, nullptr);
		deviceContext->ClearRenderTargetView(renderTargetView, color);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swapChain->Present(1U, 0U);

		//game loop
		while (GetAsyncKeyState(VK_LBUTTON)) {
			std::thread aimbotThread(getEnemyPlayers, memory, cgamex86, qagamex86, quake, 3003);
			//getEnemyPlayers(memory, cgamex86, qagamex86, quake, 3003);
		}

		while (GetAsyncKeyState(VK_XBUTTON1) && railGunTimer.elapsed() > 1.2f) {
			auto railGunShootingTimer = Timer{};
			while (railGunShootingTimer.elapsed() < 0.05) {
				getEnemyPlayers(memory, cgamex86, qagamex86, quake, 253);
			}
			railGunTimer.reset();
		}
	}
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	if (swapChain) {
		swapChain->Release();
	}

	if (deviceContext) {
		deviceContext->Release();
	}

	if (device) {
		device->Release();
	}

	if (renderTargetView) {
		renderTargetView->Release();
	}

	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
	return 1;
}

