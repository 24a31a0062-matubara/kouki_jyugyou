#include <windows.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

HWND hwnd;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12Device> device;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12DescriptorHeap> rtvHeap;
ComPtr<ID3D12Resource> renderTargets[2];
ComPtr<ID3D12CommandAllocator> commandAllocators[2];
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12Fence> fence;
HANDLE fenceEvent;
UINT64 fenceValues[2] = {};
UINT rtvDescriptorSize;
UINT frameIndex;

void InitD3D12(HWND hwnd)
{
    // DXGIファクトリ作成
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    // ハードウェアアダプタ取得
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
            break;
    }

    // デバイス作成
    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

    // コマンドキュー作成
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    // スワップチェーン作成
    DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.Width = 1280;
    swapDesc.Height = 720;
    swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> tempSwapChain;
    factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapDesc, nullptr, nullptr, &tempSwapChain);
    tempSwapChain.As(&swapChain);
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // RTVヒープ作成
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // レンダーターゲット作成
    for (UINT i = 0; i < 2; i++)
    {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += i * rtvDescriptorSize;
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
    }

    // コマンドリスト作成
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[frameIndex].Get(), nullptr, IID_PPV_ARGS(&commandList));
    commandList->Close();

    // フェンス作成
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void ClearScreen()
{
    // コマンドリストリセット
    commandAllocators[frameIndex]->Reset();
    commandList->Reset(commandAllocators[frameIndex].Get(), nullptr);

    // バリア: Present → RenderTarget
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = renderTargets[frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrier);

    // RTVハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += frameIndex * rtvDescriptorSize;

    // 画面クリア（青色）
    float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // バリア: RenderTarget → Present
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList->ResourceBarrier(1, &barrier);

    // コマンドリストクローズ & 実行
    commandList->Close();
    ID3D12CommandList* lists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, lists);

    // フェンス同期
    fenceValues[frameIndex]++;
    commandQueue->Signal(fence.Get(), fenceValues[frameIndex]);
    if (fence->GetCompletedValue() < fenceValues[frameIndex])
    {
        fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    // バッファ切替
    swapChain->Present(1, 0);
    frameIndex = swapChain->GetCurrentBackBufferIndex();
}
