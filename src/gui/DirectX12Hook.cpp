#include "DirectX12Hook.h"

namespace DirectX12Hook {

	typedef HRESULT(APIENTRY *Present12)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);

	Present12 oPresent = nullptr;

	typedef void(APIENTRY *ExecuteCommandLists)(ID3D12CommandQueue *queue, UINT NumCommandLists, ID3D12CommandList *ppCommandLists);

	ExecuteCommandLists oExecuteCommandLists = nullptr;

	namespace DirectX12Interface {
		ID3D12Device *Device = nullptr;
		ID3D12DescriptorHeap *DescriptorHeapBackBuffers;
		ID3D12DescriptorHeap *DescriptorHeapImGuiRender;
		ID3D12GraphicsCommandList *CommandList;
		ID3D12CommandQueue *CommandQueue;

		struct _FrameContext {
			ID3D12CommandAllocator *CommandAllocator;
			ID3D12Resource *Resource;
			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
		};

		uint64_t BuffersCounts = -1;
		_FrameContext *FrameContext;
	}

	bool shutdown = false;
	HWND window = nullptr;

	HRESULT APIENTRY hkPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags) {
		static bool init = false;

		if (GetAsyncKeyState(VK_INSERT) & 0x1) {
			Menu::isOpen = !Menu::isOpen;
		}

		if (GetAsyncKeyState(VK_NUMPAD1) & 0x1) {
			pybind11::scoped_interpreter guard{};
			try {
				auto testModule = pybind11::module::import("plugins.test");
				auto func = testModule.attr("main");
				func();
			} catch (pybind11::error_already_set &e) {
				MessageBoxA(nullptr, e.what(), "what", 0);
			}
		}

		if (GetAsyncKeyState(VK_NUMPAD2) & 0x1) {
			RED4ext::StackArgs_t stackArgs;
			RED4ext::ScriptGameInstance scriptGameInstance;
			stackArgs.push_back({nullptr, &scriptGameInstance});
			auto cstr1 = RED4ext::CString("-2382.430176");
			stackArgs.push_back({nullptr, &cstr1});
			auto cstr2 = RED4ext::CString("-610.183594");
			stackArgs.push_back({nullptr, &cstr2});
			auto cstr3 = RED4ext::CString("12.673874");
			stackArgs.push_back({nullptr, &cstr3});
			RED4ext::ExecuteGlobalFunction("TeleportPlayerToPosition", nullptr, stackArgs);
		}

		if (!init) {
			if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void **) &DirectX12Interface::Device))) {
				ImGui::CreateContext();

				CreateEvent(nullptr, false, false, nullptr);

				DXGI_SWAP_CHAIN_DESC sdesc;
				pSwapChain->GetDesc(&sdesc);
				sdesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
				sdesc.OutputWindow = window;
				sdesc.Windowed = (GetWindowLongPtr(window, GWL_STYLE) & WS_POPUP) == 0;

				DirectX12Interface::BuffersCounts = sdesc.BufferCount;
				DirectX12Interface::FrameContext = new DirectX12Interface::_FrameContext[DirectX12Interface::BuffersCounts];

				D3D12_DESCRIPTOR_HEAP_DESC descriptorImGuiRender = {};
				descriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				descriptorImGuiRender.NumDescriptors = DirectX12Interface::BuffersCounts;
				descriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				if (DirectX12Interface::Device->CreateDescriptorHeap(&descriptorImGuiRender, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapImGuiRender)) != S_OK)
					return false;

				ID3D12CommandAllocator *allocator;
				if (DirectX12Interface::Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)) != S_OK)
					return false;

				for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
					DirectX12Interface::FrameContext[i].CommandAllocator = allocator;
				}

				if (DirectX12Interface::Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&DirectX12Interface::CommandList)) != S_OK || DirectX12Interface::CommandList->Close() != S_OK)
					return false;

				D3D12_DESCRIPTOR_HEAP_DESC descriptorBackBuffers;
				descriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				descriptorBackBuffers.NumDescriptors = DirectX12Interface::BuffersCounts;
				descriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				descriptorBackBuffers.NodeMask = 1;

				if (DirectX12Interface::Device->CreateDescriptorHeap(&descriptorBackBuffers, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapBackBuffers)) != S_OK)
					return false;

				const auto rtvDescriptorSize = DirectX12Interface::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = DirectX12Interface::DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

				for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
					ID3D12Resource *pBackBuffer = nullptr;

					DirectX12Interface::FrameContext[i].DescriptorHandle = rtvHandle;
					pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
					DirectX12Interface::Device->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
					DirectX12Interface::FrameContext[i].Resource = pBackBuffer;
					rtvHandle.ptr += rtvDescriptorSize;
				}

				Menu::Init(window, DirectX12Interface::Device, (int) DirectX12Interface::BuffersCounts, DirectX12Interface::DescriptorHeapImGuiRender);

				InputHooks::Init(window);
			}
			init = true;
		}

		if (shutdown || DirectX12Interface::CommandQueue == nullptr) return oPresent(pSwapChain, SyncInterval, Flags);

		Menu::NewFrame();
		Menu::Update();

		auto &currentFrameContext = DirectX12Interface::FrameContext[pSwapChain->GetCurrentBackBufferIndex()];
		currentFrameContext.CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = currentFrameContext.Resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		DirectX12Interface::CommandList->Reset(currentFrameContext.CommandAllocator, nullptr);
		DirectX12Interface::CommandList->ResourceBarrier(1, &barrier);
		DirectX12Interface::CommandList->OMSetRenderTargets(1, &currentFrameContext.DescriptorHandle, FALSE, nullptr);
		DirectX12Interface::CommandList->SetDescriptorHeaps(1, &DirectX12Interface::DescriptorHeapImGuiRender);

		Menu::Render(DirectX12Interface::CommandList);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		DirectX12Interface::CommandList->ResourceBarrier(1, &barrier);
		DirectX12Interface::CommandList->Close();

		DirectX12Interface::CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList *const *>(&DirectX12Interface::CommandList));


		return oPresent(pSwapChain, SyncInterval, Flags);
	}

	void hkExecuteCommandLists(ID3D12CommandQueue *queue, UINT NumCommandLists, ID3D12CommandList *ppCommandLists) {
		if (!DirectX12Interface::CommandQueue)
			DirectX12Interface::CommandQueue = queue;

		oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
	}

	void Shutdown() {
		shutdown = true;
		DirectX12Interface::Device->Release();
		DirectX12Interface::DescriptorHeapBackBuffers->Release();
		DirectX12Interface::DescriptorHeapImGuiRender->Release();
		DirectX12Interface::CommandList->Release();
		DirectX12Interface::CommandQueue->Release();
	}

	void Init(HWND hwnd) {
		window = hwnd;
		if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
			kiero::bind(54, (void **) &oExecuteCommandLists, hkExecuteCommandLists);
			kiero::bind(140, (void **) &oPresent, hkPresent);
		}
	}
}

