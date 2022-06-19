#pragma once

#include "../stdafx.h"
#include "menu.h"
#include <MinHook.h>
#include "../cet/Addresses.h"

namespace inputhook {
	extern void Init(HWND hWindow);
	extern void Remove(HWND hWindow);
	static LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}