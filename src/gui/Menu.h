#pragma once
#include "stdafx.h"
#include "ImGuiCP.h"

namespace Menu {
	extern bool isOpen;
	extern void Init(void* hwnd, ID3D12Device* device, int num_frames_in_flight, ID3D12DescriptorHeap* cbv_srv_heap);
	extern void NewFrame();
	extern void Update();
	extern void Render(ID3D12GraphicsCommandList* ctx);
	extern void Shutdown();
};
