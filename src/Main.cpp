#include "stdafx.h"
#include "gui/DirectX12Hook.h"
#include "RedPython.h"

RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes() {
}

RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes() {
}

DWORD WINAPI InitThread(LPVOID lpParameter) {
	Sleep(1000);
	HWND window = (HWND) RED4ext::CGameEngine::Get()->unkC0->hWnd;
	DirectX12Hook::Init(window);
	return 0;
}

bool BaseInit_OnEnter(RED4ext::CGameApplication *aApp) {
	CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
	return true;
}

void messageBox(const std::string &text, const std::string &title) {
	MessageBoxA(nullptr, text.c_str(), title.c_str(), 0);
}

PYBIND11_EMBEDDED_MODULE(cyberpunk, m) {
	auto rtti = RED4ext::CRTTISystem::Get();

	// Def all globals
	RED4ext::DynArray<RED4ext::CBaseFunction*> funcs;
	rtti->GetGlobalFunctions(funcs);
	for (const auto &func : funcs) {
		const std::string cShortName = func->shortName.ToString();
		m.def(func->shortName.ToString(), [&func](const pybind11::args& pyArgs){
			return RedPython::ToPython(RedPython::ExecuteGlobalFunction(func->fullName.ToString(), pyArgs));
		});
	}

	m.def("ExecuteGlobalFunction", &RedPython::ExecuteGlobalFunction);
	m.def("GetInstance", &RedPython::GetInstance);
	m.def("messageBox", &messageBox);
	m.def("ToPython", &RedPython::ToPython);
	pybind11::class_<RED4ext::CStackType>(m, "CStackType", pybind11::dynamic_attr())
			.def("GetPropValue", [](const pybind11::object& self, const std::string& propName){
				return RedPython::ToPython(RedPython::GetPropValue(pybind11::cast<RED4ext::CStackType>(self), propName));
			})
			.def("SetPropValue", [](const pybind11::object& self, const std::string& propName, const pybind11::object& value){
				RedPython::SetPropValue(pybind11::cast<RED4ext::CStackType>(self), propName, value);
			})
			.def("ExecuteFunction", [](const pybind11::object& self, const std::string& funcName, const pybind11::tuple& args){
				return RedPython::ToPython(RedPython::ExecuteFunction(pybind11::cast<RED4ext::CStackType>(self), funcName, args));
			});
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
