#include <Windows.h>
#include <RED4ext/Relocation.hpp>
#include <RED4ext/DynArray.hpp>
#include <RED4ext/CName.hpp>
#include <RED4ext/CString.hpp>
#include <RED4ext/ISerializable.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/GameEngine.hpp>
#include <RED4ext/Scripting/Stack.hpp>
#include <RED4ext/Scripting/CProperty.hpp>
#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/OpcodeHandlers.hpp>
#include <RED4ext/TweakDB.hpp>
#include <tchar.h>
#include <dxgi1_4.h>
#include <MinHook.h>
#include <Psapi.h>
#include <fstream>
#include "kiero.h"
#include "RED4ext/Scripting/Natives/GameTime.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include <dxgi.h>
#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")

void testTime() {
	RED4ext::GameTime gameTime;
	RED4ext::ExecuteFunction("gameTimeSystem", "GetGameTime", &gameTime);

	gameTime.AddHours(10);
	gameTime.AddMinutes(30);
	gameTime.AddSeconds(15);

	RED4ext::ExecuteFunction("gameTimeSystem", "SetGameTimeBySeconds", nullptr, gameTime.ToSeconds());
}

PYBIND11_EMBEDDED_MODULE(cyberpunk, m) {
	m.def("testTime", &testTime);
}

typedef long(__fastcall *PresentD3D12)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);

PresentD3D12 oPresentD3D12;

long __fastcall hookPresentD3D12(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags) {

	if (GetAsyncKeyState(VK_NUMPAD1) & 0x1) {
		pybind11::scoped_interpreter guard{};
		try {
			auto testModule = pybind11::module::import("plugins.test");
			auto func = testModule.attr("main");
			func();
		} catch (pybind11::error_already_set &e) {
			MessageBoxA(0, e.what(), "what", 0);
		}
//		pybind11::scoped_interpreter guard{};
//		pybind11::exec("import os\nos.system('mspaint')");
//		Py_Initialize();
//		PyRun_SimpleString("import os\nos.system('mspaint')");
//		Py_Finalize();
	}

	if (GetAsyncKeyState(VK_NUMPAD2) & 0x1) {
		testTime();
	}

	if (GetAsyncKeyState(VK_NUMPAD3) & 0x1) {
		RED4ext::GameTime gameTime;
		RED4ext::ExecuteFunction("gameTimeSystem", "GetGameTime", &gameTime);
		MessageBoxA(nullptr, gameTime.ToString().c_str(), "VK_NUMPAD1", 0);
	}

	if (GetAsyncKeyState(VK_NUMPAD4) & 0x1) {
		pybind11::scoped_interpreter guard{};
		pybind11::exec("import cyberpunk\ncyberpunk.testTime()");
	}

	return oPresentD3D12(pSwapChain, SyncInterval, Flags);
}

bool IsCyberpunk(HWND ahWnd) {
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(ahWnd, &lpdwProcessId);
	if (lpdwProcessId == GetCurrentProcessId()) {
		TCHAR name[512] = {0};
		GetWindowText(ahWnd, name, 511);
		if (_tcscmp(_T("Cyberpunk 2077 (C) 2020 by CD Projekt RED"), name) == 0) {
			return true;
		}
	}
	return false;
}


DWORD WINAPI MainThread(LPVOID lpParameter) {
	Sleep(5000);
	HWND hwnd = nullptr;
	while (hwnd == nullptr) {
		HWND ForegroundWindow = GetForegroundWindow();
		if (IsCyberpunk(ForegroundWindow)) {
			hwnd = ForegroundWindow;
		}
	}
	if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
		kiero::bind(140, (void **) &oPresentD3D12, hookPresentD3D12);
	}
//	Py_Initialize();
//	PyRun_SimpleString("import os\nos.system('mspaint')");
//	Py_Finalize();
//	MessageBoxA(nullptr, "test1", "test2", 0);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
//		auto* pRtti = RED4ext::CRTTISystem::Get();
//		auto* pType = pRtti->GetType(RED4ext::FNV1a64("test"));
//		Py_Initialize();
//		PyRun_SimpleString("import os\nos.system('mspaint')");
//		Py_Finalize();
//		MessageBoxA(nullptr, "test1", "test2", 0);
	} else if (fdwReason == DLL_PROCESS_DETACH) {
		FreeLibraryAndExitThread(hinstDLL, TRUE);
	}
	return TRUE;
}