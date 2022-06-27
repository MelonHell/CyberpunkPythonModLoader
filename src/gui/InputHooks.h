#pragma once

#include "../stdafx.h"
#include <MinHook.h>
#include "../cet/Addresses.h"
#include "Menu.h"

namespace InputHooks {
	extern void Init(HWND hWindow);
	extern void Remove(HWND hWindow);
	static LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}