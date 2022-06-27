#pragma once

#include <Windows.h>
#include <tchar.h>
#include "libs/kiero.h"

// Red4ext
#include <RED4ext/RED4ext.hpp>
#include "RED4ext/Scripting/Natives/ScriptGameInstance.hpp"

// python
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

// dx shit
#include <dxgi.h>
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_4.h>

// imgui
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_win32.h"
#include "libs/imgui/imgui_impl_dx12.h"