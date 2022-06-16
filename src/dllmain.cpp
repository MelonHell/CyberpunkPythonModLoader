#include <Windows.h>
#include <RED4ext/Relocation.hpp>
#include <RED4ext/DynArray.hpp>
#include <RED4ext/ISerializable.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/GameEngine.hpp>
#include <RED4ext/Scripting/Stack.hpp>
#include <RED4ext/Scripting/CProperty.hpp>
#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/TweakDB.hpp>
#include <tchar.h>
#include <dxgi1_4.h>
#include "kiero.h"
#include "RED4ext/Scripting/Natives/GameTime.hpp"
#include "RED4ext/Scripting/Natives/ScriptGameInstance.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <RED4ext/Scripting/Utils.hpp>

#include <dxgi.h>

#pragma comment(lib, "d3d12.lib")

namespace py = pybind11;


struct RedAnyTypeValue;

RedAnyTypeValue ExecuteFunction(RED4ext::ScriptInstance aInstance, RED4ext::CBaseFunction *aFunc, RED4ext::StackArgs_t aArgs);

RedAnyTypeValue executeFunction(RedAnyTypeValue object, std::string funcName);
py::object toPythonValue(RedAnyTypeValue redAnyTypeValue);
RedAnyTypeValue GetPropValue(RedAnyTypeValue redAnyTypeValue, std::string propertyName);

struct RedAnyTypeValue {
	RedAnyTypeValue(RED4ext::CBaseRTTIType *type, RED4ext::ScriptInstance value) : type(type), value(value) {}

	RED4ext::CBaseRTTIType *type;
	RED4ext::ScriptInstance value;

	py::object execute(std::string funcName) {
		return toPythonValue(executeFunction(*this, funcName));
	}

	py::object get(std::string propName) {
		return toPythonValue(GetPropValue(*this, propName));
	}
};

RedAnyTypeValue unHandle(RED4ext::CStackType object) {
	if (object.type->GetType() == RED4ext::ERTTIType::Handle) {
		RED4ext::Handle<RED4ext::IScriptable> handle = *static_cast<RED4ext::Handle<RED4ext::IScriptable> *>(object.value);
		return {handle->GetType(), handle.GetPtr()};
	}
	return {object.type, object.value};
}

RedAnyTypeValue ExecuteFunction(RED4ext::ScriptInstance aInstance, RED4ext::CBaseFunction *aFunc, RED4ext::StackArgs_t aArgs) {
	RED4ext::CStackType result;
	if (aFunc->returnType) {
		result.type = aFunc->returnType->type;
		auto allocator = aFunc->returnType->type->GetAllocator();
		auto allocResult = allocator->Alloc(aFunc->returnType->type->GetSize());
		aFunc->returnType->type->Construct(allocResult.memory);
		result.value = allocResult.memory;
	} else {
		result = nullptr;
	}
	for (size_t i = 0; i < aArgs.size() && i < aFunc->params.size; i++) {
		auto &arg = aArgs[i];
		if (!arg.type) arg.type = aFunc->params[static_cast<uint32_t>(i)]->type;
	}
	RED4ext::CStack stack(aInstance, aArgs.data(), static_cast<uint32_t>(aArgs.size()), &result);
	aFunc->Execute(&stack);
	return unHandle(result);
}

RedAnyTypeValue executeFunctionWithArgs(RedAnyTypeValue object, std::string funcName, RED4ext::StackArgs_t aArgs = {}) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto cls = rtti->GetClass(object.type->GetName());
	auto aFunc = cls->GetFunction(funcName.c_str());
	return ExecuteFunction(object.value, aFunc, aArgs);
}

RedAnyTypeValue executeFunction(RedAnyTypeValue object, std::string funcName) {
	return executeFunctionWithArgs(object, funcName, {});
}

RedAnyTypeValue executeGlobalFunction(std::string funcName) {
	RED4ext::ScriptGameInstance gameInstance;
	auto rtti = RED4ext::CRTTISystem::Get();
	RED4ext::Handle<RED4ext::IScriptable> handle;
	RED4ext::StackArgs_t args;
	((args.emplace_back(nullptr, &gameInstance)));
	auto func = rtti->GetFunction((funcName + ";GameInstance").c_str());
	auto engine = RED4ext::CGameEngine::Get();
	auto game = engine->framework->gameInstance;
	RED4ext::CStackType object;
	object.type = rtti->GetClass("cpPlayerSystem");
	RED4ext::Handle<RED4ext::IScriptable> instance(game->GetInstance(object.type));
	object.value = instance;
	return ExecuteFunction(instance, func, args);
}

RedAnyTypeValue GetValuePtr(RED4ext::CProperty *prop, RED4ext::ScriptInstance aInstance) {
	void *holder = aInstance;
	if (prop->flags.b21) {
		/*auto scriptable = static_cast<IScriptable*>(aInstance);
		holder = scriptable->GetValueHolder();*/

		using func_t = void *(*)(RED4ext::ScriptInstance);
		RED4ext::RelocFunc<func_t> func(RED4ext::Addresses::IScriptable_GetValueHolder);
		holder = func(aInstance);
	}

	void *value = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(holder) + prop->valueOffset);
	return {prop->type, value};
}

RedAnyTypeValue GetPropValue(RedAnyTypeValue redAnyTypeValue, std::string propertyName) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto playerPuppetCls = rtti->GetClass(redAnyTypeValue.type->GetName());
	auto inCrouch = playerPuppetCls->GetProperty(propertyName.c_str());
	return GetValuePtr(inCrouch, redAnyTypeValue.value);
}

float getFloat(RedAnyTypeValue redAnyTypeValue, std::string propertyName) {
	auto rtti = RED4ext::CRTTISystem::Get();
	auto playerPuppetCls = rtti->GetClass(redAnyTypeValue.type->GetName());
	auto inCrouch = playerPuppetCls->GetProperty(propertyName.c_str());
	auto value = inCrouch->GetValue<float>(redAnyTypeValue.value);
	return value;
}

py::object toPythonValue(RedAnyTypeValue redAnyTypeValue) {
	if (redAnyTypeValue.type->GetName() == "Float") return py::cast(*reinterpret_cast<float *>(redAnyTypeValue.value));
	MessageBoxA(0, redAnyTypeValue.type->GetName().ToString(), "type", 0);
	return py::cast(redAnyTypeValue);
}

void messageBox(const std::string &text, const std::string &title) {
	MessageBoxA(nullptr, text.c_str(), title.c_str(), 0);
}


PYBIND11_EMBEDDED_MODULE(cyberpunk, m) {
	m.def("ExecuteFunction", executeFunction);
	m.def("ExecuteGlobalFunction", executeGlobalFunction);
	m.def("GetPropValue", &GetPropValue);
	m.def("messageBox", &messageBox);
	m.def("getFloat", &getFloat);
	py::class_<RedAnyTypeValue>(m, "RedAnyTypeValue")
	        .def("execute", &RedAnyTypeValue::execute)
			.def("get", &RedAnyTypeValue::get);
	py::class_<RED4ext::Handle<RED4ext::IScriptable>>(m, "IScriptableHandle");
	py::class_<RED4ext::CStackType>(m, "CStackType");
}

typedef long(__fastcall *PresentD3D12)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);

PresentD3D12 oPresentD3D12;

long __fastcall hookPresentD3D12(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags) {

	if (GetAsyncKeyState(VK_NUMPAD1) & 0x1) {
		py::scoped_interpreter guard{};
		try {
			auto testModule = py::module::import("plugins.test");
			auto func = testModule.attr("main");
			func();
		} catch (py::error_already_set &e) {
			MessageBoxA(0, e.what(), "what", 0);
		}
//		py::scoped_interpreter guard{};
//		py::exec("import os\nos.system('mspaint')");
//		Py_Initialize();
//		PyRun_SimpleString("import os\nos.system('mspaint')");
//		Py_Finalize();
	}

	if (GetAsyncKeyState(VK_NUMPAD2) & 0x1) {
//		testTime();
//		test();
	}

	if (GetAsyncKeyState(VK_NUMPAD3) & 0x1) {
		RED4ext::GameTime gameTime;
		RED4ext::ExecuteFunction("gameTimeSystem", "GetGameTime", &gameTime);
		MessageBoxA(nullptr, gameTime.ToString().c_str(), "VK_NUMPAD1", 0);
	}

	if (GetAsyncKeyState(VK_NUMPAD4) & 0x1) {
		py::scoped_interpreter guard{};
		py::exec("import cyberpunk\ncyberpunk.testTime()");
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