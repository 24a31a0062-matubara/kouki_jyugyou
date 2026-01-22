#include "global.h"
#include <d3d12.h>
#include <dxgi1_6.h>

ID3D12Device* g_device = nullptr;
ID3D12GraphicsCommandList* g_commandList = nullptr;
ID3D12CommandQueue* g_commandQueue = nullptr;
IDXGISwapChain3* g_swapChain = nullptr;
ID3D12DescriptorHeap* g_rtvHeap = nullptr;
UINT g_rtvDescriptorSize = 0;
UINT g_frameIndex = 0;
ID3D12Resource* g_renderTargets[2] = {};
ID3D12CommandAllocator* g_commandAllocator = nullptr;
HANDLE g_fenceEvent = nullptr;
ID3D12Fence* g_fence = nullptr;
UINT64 g_fenceValue = 0;

UINT g_windowWidth = 1280;
UINT g_windowHeight = 720;
