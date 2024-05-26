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
#include <thread>
#include "ql_hack.h"
#include <random>

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

static int endProcess(IDXGISwapChain*& swapChain, ID3D11DeviceContext*& deviceContext, ID3D11Device*& device, ID3D11RenderTargetView*& renderTargetView, HWND window,
	LPCWSTR lpszClassName, HINSTANCE hInstance) {
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
	UnregisterClassW(lpszClassName, hInstance);
	return 1;
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

	auto qlHack = QLHack{ true, true, 8, 20 };

	auto railGunTimer = Timer{};

	bool running = true;

	while (running) {
		qlHack.GetEnemyEntities();
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

		if (qlHack.wallhack) {
			const auto positions = qlHack.GetEnemiesPosition();
			for (Vector3 position : positions) {
				const auto screenCoords = position.WorldToScreen(qlHack.GetRefDef());
				ImGui::GetBackgroundDrawList()->AddRect({ screenCoords.x - 10.f, screenCoords.y - 10.f },
					{ screenCoords.x + 20.f, screenCoords.y + 50.f }, ImColor(1.f, 0.f, 0.f));
			}
		}

		ImGui::Render();

		constexpr float color[4]{ 0.f, 0.f, 0.f, 0.f };
		deviceContext->OMSetRenderTargets(1U, &renderTargetView, nullptr);
		if (renderTargetView == 0) {
			endProcess(swapChain, deviceContext, device, renderTargetView, window, wc.lpszClassName, wc.hInstance);
		}
		deviceContext->ClearRenderTargetView(renderTargetView, color);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swapChain->Present(1U, 0U);

		//game loop
		if (GetAsyncKeyState(VK_XBUTTON2) && qlHack.aimbot) {
			if (qlHack.railGunTimer.elapsed() > 0.3) {
				qlHack.GetEnemyPlayers();
			}

		}
	}

	return endProcess(swapChain, deviceContext, device, renderTargetView, window, wc.lpszClassName, wc.hInstance);
}
