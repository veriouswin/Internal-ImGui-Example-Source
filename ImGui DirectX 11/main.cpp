#include "includes.h"

#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <direct.h>
#include <random>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <d3d11.h>
#include "functional"
#include "filesystem"
#include <d3d11.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <Wininet.h>
#include <string>
#include <io.h>
#include "font_awesome.cpp"
#include "imgui/imgui_internal.h"
#include "settings.h"
#include "main.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
HMODULE hModule;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	//io.Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 }; //ICON_MIN_FA , ICON_MAX_FA
	ImFontConfig icons_config;

	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 1.0;
	icons_config.OversampleV = 1.0;

	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.WantCaptureKeyboard;
	io.WantCaptureMouse;
	io.FontAllowUserScaling;

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 15.0f);
	io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 15.0f, &icons_config, icons_ranges);

	ImGui_ImplWin32_Init(window); //window
	ImGui_ImplDX11_Init(pDevice, pContext);

	int LogoWidth = 16;
	int LogoHeight = 16;
}

bool showui = true;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (showui)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

float CenterX = GetSystemMetrics(SM_CXSCREEN) / 2;
float CenterY = GetSystemMetrics(SM_CYSCREEN) / 2;

ImColor FovCircleColor = ImColor(255, 255, 255, 255);
static float AimbotFov[4] = { 1.f, 1.f, 1.f, 1.f };

void RenderESP()
{
	if (Settings::aimfov == true)
	{
		const auto cur_window1 = ImGui::GetForegroundDrawList();
		cur_window1->AddCircle(ImVec2(CenterX, CenterY), Settings::aimfovvalue, FovCircleColor, 100, 1.f);
	}
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1) {
		showui = !showui;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	RenderESP();

	ImGui::GetMouseCursor();
	ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	ImGui::GetIO().WantCaptureMouse = showui;
	ImGui::GetIO().MouseDrawCursor = showui;
	
	if (showui)
	{
		ImGui::SetNextWindowSize({ 400, 300 });
		ImGui::Begin("verious.win", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		const auto cur_window1 = ImGui::GetBackgroundDrawList();

		if (showui == true)
		{
			cur_window1->AddRectFilled(ImVec2(0 + 1, 0 + 1), ImVec2(0 + 2700, 0 + 1200), IM_COL32(10, 10, 10, 85));
		}

		if(ImGui::Button(ICON_FA_CROSSHAIRS " Aiming", { 120, 30}))
		{
			Settings::tabs = 0;
		}
		ImGui::SameLine();	
		if (ImGui::Button(ICON_FA_EYE " Visuals", { 120, 30 }))
		{
			Settings::tabs = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_GEARS " Settings", { 120, 30 }))
		{
			Settings::tabs = 2;
		}

		if(Settings::tabs == 0)
		{
			ImGui::BeginChild("##child1", ImVec2(ImGui::GetContentRegionAvail().x / 1, ImGui::GetContentRegionAvail().y / 1), true, ImGuiWindowFlags_NoResize);

			ImGui::SetCursorPos({ 5,5 });
			ImGui::Checkbox("Aimbot", &Settings::aimbot);

			ImGui::SetCursorPos({ 5,35 });
			ImGui::Checkbox("Aim FOV", &Settings::aimfov);

			ImGui::SameLine();
			ImGui::ColorEdit4("##color", AimbotFov, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
			FovCircleColor.Value.x = AimbotFov[0];
			FovCircleColor.Value.y = AimbotFov[1];
			FovCircleColor.Value.z = AimbotFov[2];
			FovCircleColor.Value.w = AimbotFov[3];

			ImGui::SetCursorPos({ 5,65 });
			ImGui::SliderFloat("FOV", &Settings::aimfovvalue, 10.f, 800.f);

			/*ImGui::SetCursorPos({ 5,95 });
			ImGui::Checkbox("Spinbot [CAPS]", &Settings::spinbot);*/

			ImGui::EndChild();
		} else if (Settings::tabs == 1)
		{
			ImGui::BeginChild("##child2", ImVec2(ImGui::GetContentRegionAvail().x / 1, ImGui::GetContentRegionAvail().y / 1), true, ImGuiWindowFlags_NoResize);

			ImGui::SetCursorPos({ 5,5 });
			ImGui::Text("2");

			ImGui::EndChild();
		} else if (Settings::tabs == 2)
		{
			ImGui::BeginChild("##child3", ImVec2(ImGui::GetContentRegionAvail().x / 1, ImGui::GetContentRegionAvail().y / 1), true, ImGuiWindowFlags_NoResize);

			ImGui::SetCursorPos({ 5,5 });
			ImGui::Text("3");

			ImGui::EndChild();
		}

		/*if (ImGui::Button("Unhook", { 120, 30 }))
		{
			kiero::shutdown();
			FreeLibrary(hModule);
		}*/

		ImGui::End();
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);

	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}

	return TRUE;
}
