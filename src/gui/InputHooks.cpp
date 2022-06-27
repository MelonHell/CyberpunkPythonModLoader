#include "InputHooks.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace ImGui;
using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0*);

namespace InputHooks {
	WNDPROC oWndProc;
	TClipToCenter* m_realClipToCenter{ nullptr };

	BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis)
	{
		HWND wnd = (HWND)apThis->hWnd;
		HWND foreground = GetForegroundWindow();

		if (wnd == foreground && apThis->unk164 && !apThis->unk154 && !Menu::isOpen)
		{
			RECT rect;
			GetClientRect(wnd, &rect);
			ClientToScreen(wnd, reinterpret_cast<POINT*>(&rect.left));
			ClientToScreen(wnd, reinterpret_cast<POINT*>(&rect.right));
			rect.left = (rect.left + rect.right) / 2;
			rect.right = rect.left;
			rect.bottom = (rect.bottom + rect.top) / 2;
			rect.top = rect.bottom;
			apThis->isClipped = true;
			ShowCursor(FALSE);
			return ClipCursor(&rect);
		}

		if(apThis->isClipped)
		{
			apThis->isClipped = false;
			return ClipCursor(nullptr);
		}

		return 1;
	}


	void Init(HWND hWindow) {
		oWndProc = (WNDPROC) SetWindowLongPtr(hWindow, GWLP_WNDPROC, (__int3264) (LONG_PTR) WndProc);
		RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CWinapi_ClipToCenter);

		uint8_t* pLocation = func.GetAddr();

		if (pLocation) {
			MH_CreateHook(pLocation, &ClipToCenter, reinterpret_cast<void**>(&m_realClipToCenter));
			MH_EnableHook(pLocation);
		}
	}

	void Remove(HWND hWindow) {
		SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR) oWndProc);
	}

	LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (Menu::isOpen) {
			ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
			// ignore mouse & keyboard events
			if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)) return 0;
			// ignore specific messages
			if (uMsg == WM_INPUT) return 0;
		}

		return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
	}
}

