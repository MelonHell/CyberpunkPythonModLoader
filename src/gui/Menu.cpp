#include "Menu.h"

namespace Menu {
	bool isOpen = false;

	void Init(void *hwnd, ID3D12Device *device, int num_frames_in_flight, ID3D12DescriptorHeap *cbv_srv_heap) {
		ImGui::StyleColorsDark();
		ImGui::GetIO().Fonts->AddFontDefault();
		ImGuiCP::Init();

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX12_Init(device, num_frames_in_flight, DXGI_FORMAT_R8G8B8A8_UNORM, cbv_srv_heap, cbv_srv_heap->GetCPUDescriptorHandleForHeapStart(), cbv_srv_heap->GetGPUDescriptorHandleForHeapStart());
		ImGui_ImplDX12_CreateDeviceObjects();
	}

	void NewFrame() {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void Update() {
		ImGui::GetIO().MouseDrawCursor = isOpen;
		if (isOpen) {
			ImGui::Begin("Test shit");
			ImGuiCP::Button("test");
			ImGui::End();
		}
	}

	void Render(ID3D12GraphicsCommandList *ctx) {
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), ctx);
	}

	void Shutdown() {
		ImGui_ImplDX12_Shutdown();
		ImGui::DestroyContext();
	}
}