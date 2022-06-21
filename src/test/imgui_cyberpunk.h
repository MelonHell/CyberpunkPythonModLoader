#pragma once
#include "../stdafx.h"

namespace ImGuiCP {
	extern ImFont* rajdhani32;
	extern void InitFonts();
	extern bool Button(const char* label);
	extern bool InputText(const char *label, char *buf, size_t buf_size);
}

