#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>

extern int g_windowWidth;
extern int g_windowHeight;

extern ID3D12Device* g_device;
extern ID3D12CommandQueue* g_commandQueue;
extern IDXGISwapChain3* g_swapChain;
extern ID3D12DescriptorHeap* g_rtvHeap;
extern ID3D12Resource* g_renderTargets[2];
extern ID3D12CommandAllocator* g_commandAllocator;
extern ID3D12GraphicsCommandList* g_commandList;
extern ID3D12Fence* g_fence;
extern HANDLE g_fenceEvent;
extern UINT64 g_fenceValue;
extern UINT g_frameIndex;
extern UINT g_rtvDescriptorSize;
