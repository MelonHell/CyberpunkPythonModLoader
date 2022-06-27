#include "stdafx.h"
#include "test/d3d12hook.h"
#include "globals.h"

RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes() {
}

RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes() {
}

DWORD WINAPI InitThread(LPVOID lpParameter) {
	Sleep(1000);
	globals::mainWindow = (HWND) RED4ext::CGameEngine::Get()->unkC0->hWnd;
	if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
		kiero::bind(54, (void **) &d3d12hook::oExecuteCommandListsD3D12, d3d12hook::hookExecuteCommandListsD3D12);
		kiero::bind(140, (void **) &d3d12hook::oPresentD3D12, d3d12hook::hookPresentD3D12);
	}
	return 0;
}

bool BaseInit_OnEnter(RED4ext::CGameApplication *aApp) {
	CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
	return true;
}

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason, const RED4ext::Sdk *aSdk) {
	if (aReason == RED4ext::EMainReason::Load) {
		pybind11::scoped_interpreter guard{};
		pybind11::print("Hello from python");

		RED4ext::GameState initState;
		initState.OnEnter = &BaseInit_OnEnter;
		initState.OnUpdate = nullptr;
		initState.OnExit = nullptr;
		aSdk->gameStates->Add(aHandle, RED4ext::EGameStateType::Initialization, &initState);

		RED4ext::RTTIRegistrator::Add(RegisterTypes, PostRegisterTypes);
	}

	if (aReason == RED4ext::EMainReason::Unload) {

	}

	return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo *aInfo) {
	aInfo->name = L"CyberpunkPythonModLoader";
	aInfo->author = L"MelonHell";
	aInfo->version = RED4EXT_SEMVER(1, 0, 0);
	aInfo->runtime = RED4EXT_RUNTIME_LATEST;
	aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports() {
	return RED4EXT_API_VERSION_LATEST;
}
