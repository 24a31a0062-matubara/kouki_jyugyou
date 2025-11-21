#include <windows.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

IDXGIFactory4* CreateDXGIFactory()
{
    IDXGIFactory4* factory;
    UINT createFactoryFlags = 0;

#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    HRESULT hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory));
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create DXGI Factory\n");
        return nullptr;
    }

    return factory;
}

IDXGIAdapter1* GetHardwareAdapter(IDXGIFactory4* factory)
{
    IDXGIAdapter1* adapter = nullptr;

    for (UINT adapterIndex = 0; ; ++adapterIndex)
    {
        if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
        {
            break;
        }

        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            adapter->Release();
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
        {
            return adapter;
        }

        adapter->Release();
    }

    return nullptr;
}

ID3D12Device* CreateD3D12Device(IDXGIAdapter1* adapter)
{
    ID3D12Device* device;

    HRESULT hr = D3D12CreateDevice(
        adapter,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device)
    );

    if (FAILED(hr))
    {
        hr = D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device)
        );

        if (FAILED(hr))
        {
            OutputDebugString(L"Failed to create D3D12 Device\n");
            return nullptr;
        }

        OutputDebugString(L"Using software adapter (WARP)\n");
    }

    return device;
}

ID3D12CommandQueue* CreateCommandQueue(ID3D12Device* device)
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    ID3D12CommandQueue* commandQueue;
    HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create Command Queue\n");
        return nullptr;
    }

    commandQueue->SetName(L"Main Command Queue");

    return commandQueue;
}

IDXGISwapChain3* CreateSwapChain(IDXGIFactory4* factory, ID3D12CommandQueue* commandQueue, HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = 1280;
    swapChainDesc.Height = 720;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    IDXGISwapChain1* swapChain1;
    HRESULT hr = factory->CreateSwapChainForHwnd(
        commandQueue,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );

    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create Swap Chain\n");
        return nullptr;
    }

    IDXGISwapChain3* swapChain;
    hr = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
    swapChain1->Release();

    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to cast to SwapChain3\n");
        return nullptr;
    }

    return swapChain;
}

void EnableDebugLayer()
{
#if defined(_DEBUG)
    ID3D12Debug* debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();

        ID3D12Debug1* debugController1;
        if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1))))
        {
            debugController1->SetEnableGPUBasedValidation(TRUE);
        }
    }
#endif
}
