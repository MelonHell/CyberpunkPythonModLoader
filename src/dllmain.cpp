#include "stdafx.h"

#include "PyGameObject.h"

void messageBox(const std::string &text, const std::string &title) {
	MessageBoxA(nullptr, text.c_str(), title.c_str(), 0);
}

std::vector<std::string> getGlobalFuncs() {
	auto rtti = RED4ext::CRTTISystem::Get();
	RED4ext::DynArray<RED4ext::CBaseFunction*> funcs;
	rtti->GetGlobalFunctions(funcs);
	std::vector<std::string> result;
	for (const auto &item : funcs) {
		result.emplace_back(item->fullName.ToString());
	}
	return result;
}

PYBIND11_EMBEDDED_MODULE(cyberpunk, m) {

	auto rtti = RED4ext::CRTTISystem::Get();
	RED4ext::DynArray<RED4ext::CBaseFunction*> funcs;
	rtti->GetGlobalFunctions(funcs);
	for (const auto &item : funcs) {
		const std::string cShortName = item->shortName.ToString();
		m.def(item->shortName.ToString(), [&item](const pybind11::args& args){
			std::vector<pybind11::object> pyArgs;
			for (const auto &arg : args) {
				pyArgs.push_back(arg.cast<pybind11::object>());
			}
			return ExecuteGlobalFunction(item->fullName.ToString(), pyArgs);
		});
	}

	m.def("ExecuteGlobalFunction", ExecuteGlobalFunction);
	m.def("GetInstance", GetInstance);
	m.def("messageBox", &messageBox);
	m.def("getGlobalFuncs", &getGlobalFuncs);
	pybind11::class_<PyGameObject>(m, "GameObject")
	        .def("exec", &PyGameObject::exec)
			.def("get", &PyGameObject::get)
			.def("set", &PyGameObject::set)
			.def("getTypeName", &PyGameObject::getTypeName)
			.def("getTypeType", &PyGameObject::getTypeType)
			.def("getFuncs", &PyGameObject::getFuncs);
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