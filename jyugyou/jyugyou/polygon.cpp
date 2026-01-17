#include "polygon.h"
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

using namespace DirectX;

extern ID3D12Device* g_device;
extern ID3D12GraphicsCommandList* g_commandList;
extern ID3D12CommandQueue* g_commandQueue;
extern IDXGISwapChain3* g_swapChain;
extern ID3D12DescriptorHeap* g_rtvHeap;
extern UINT                    g_rtvDescriptorSize;
extern UINT                    g_frameIndex;
extern ID3D12Resource* g_renderTargets[2];
extern ID3D12CommandAllocator* g_commandAllocator;
extern HANDLE                  g_fenceEvent;
extern ID3D12Fence* g_fence;
extern UINT64                  g_fenceValue;
extern UINT                    g_windowWidth;
extern UINT                    g_windowHeight;

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position;  // 頂点座標 (x, y, z)
    XMFLOAT4 color;     // 頂点色 (r, g, b, a)
};

// 頂点データ
Vertex g_triangleVertices[] =
{
    { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // 赤
    { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 緑
    { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }  // 青
};

// インデックスデータ
UINT16 g_triangleIndices[] = { 0, 1, 2 };

// バッファとビュー
ID3D12Resource* g_vertexBuffer = nullptr;
ID3D12Resource* g_indexBuffer = nullptr;
D3D12_VERTEX_BUFFER_VIEW  g_vertexBufferView{};
D3D12_INDEX_BUFFER_VIEW   g_indexBufferView{};

// シェーダとルートシグネチャ、PSO
ID3DBlob* g_vsBlob = nullptr;
ID3DBlob* g_psBlob = nullptr;
ID3DBlob* g_errorBlob = nullptr;
ID3D12RootSignature* g_rootSignature = nullptr;
ID3D12PipelineState* g_pipelineState = nullptr;

// フェンス待ち（既にあるならそれを使ってOK）
void WaitForPreviousFrame()
{
    const UINT64 fence = g_fenceValue;
    g_commandQueue->Signal(g_fence, fence);
    g_fenceValue++;

    if (g_fence->GetCompletedValue() < fence)
    {
        g_fence->SetEventOnCompletion(fence, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }

    g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();
}

// 頂点バッファ・インデックスバッファ作成
void CreateTriangleBuffers()
{
    // 頂点バッファ
    {
        D3D12_HEAP_PROPERTIES heapProp{};
        heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProp.CreationNodeMask = 1;
        heapProp.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resDesc{};
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Alignment = 0;
        resDesc.Width = sizeof(g_triangleVertices);
        resDesc.Height = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = g_device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&g_vertexBuffer)
        );
        assert(SUCCEEDED(hr));

        // データ転送
        Vertex* mapped = nullptr;
        hr = g_vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
        assert(SUCCEEDED(hr));
        memcpy(mapped, g_triangleVertices, sizeof(g_triangleVertices));
        g_vertexBuffer->Unmap(0, nullptr);

        // ビュー設定
        g_vertexBufferView.BufferLocation = g_vertexBuffer->GetGPUVirtualAddress();
        g_vertexBufferView.StrideInBytes = sizeof(Vertex);
        g_vertexBufferView.SizeInBytes = sizeof(g_triangleVertices);
    }

    // インデックスバッファ
    {
        D3D12_HEAP_PROPERTIES heapProp{};
        heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProp.CreationNodeMask = 1;
        heapProp.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resDesc{};
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Alignment = 0;
        resDesc.Width = sizeof(g_triangleIndices);
        resDesc.Height = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = g_device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&g_indexBuffer)
        );
        assert(SUCCEEDED(hr));

        // データ転送
        void* mapped = nullptr;
        hr = g_indexBuffer->Map(0, nullptr, &mapped);
        assert(SUCCEEDED(hr));
        memcpy(mapped, g_triangleIndices, sizeof(g_triangleIndices));
        g_indexBuffer->Unmap(0, nullptr);

        // ビュー設定
        g_indexBufferView.BufferLocation = g_indexBuffer->GetGPUVirtualAddress();
        g_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        g_indexBufferView.SizeInBytes = sizeof(g_triangleIndices);
    }
}

// シェーダ・ルートシグネチャ・PSO作成
void CreatePipeline()
{
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 頂点シェーダ
    HRESULT hr = D3DCompileFromFile(
        L"VertexShader.hlsl",
        nullptr,
        nullptr,
        "vs",
        "vs_5_0",
        compileFlags,
        0,
        &g_vsBlob,
        &g_errorBlob
    );
    if (FAILED(hr))
    {
        if (g_errorBlob)
        {
            OutputDebugStringA((char*)g_errorBlob->GetBufferPointer());
            g_errorBlob->Release();
            g_errorBlob = nullptr;
        }
        assert(false);
    }

    // ピクセルシェーダ
    hr = D3DCompileFromFile(
        L"PixelShader.hlsl",
        nullptr,
        nullptr,
        "ps",
        "ps_5_0",
        compileFlags,
        0,
        &g_psBlob,
        &g_errorBlob
    );
    if (FAILED(hr))
    {
        if (g_errorBlob)
        {
            OutputDebugStringA((char*)g_errorBlob->GetBufferPointer());
            g_errorBlob->Release();
            g_errorBlob = nullptr;
        }
        assert(false);
    }

    if (g_errorBlob)
    {
        g_errorBlob->Release();
        g_errorBlob = nullptr;
    }

    // ルートシグネチャ
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* signature = nullptr;
    ID3DBlob* error = nullptr;
    hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );
    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
            error->Release();
        }
        assert(false);
    }

    hr = g_device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&g_rootSignature)
    );
    signature->Release();
    assert(SUCCEEDED(hr));

    // 入力レイアウト
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };

    // PSO 設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = g_rootSignature;
    psoDesc.VS = { g_vsBlob->GetBufferPointer(), g_vsBlob->GetBufferSize() };
    psoDesc.PS = { g_psBlob->GetBufferPointer(), g_psBlob->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    hr = g_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState));
    assert(SUCCEEDED(hr));
}

// 描画
void DrawTriangleFrame()
{
    // コマンドアロケータ・リストリセット
    g_commandAllocator->Reset();
    g_commandList->Reset(g_commandAllocator, g_pipelineState);

    // RTV ハンドル
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += g_frameIndex * g_rtvDescriptorSize;

    // レンダーターゲット設定
    g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // 画面クリア
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ビューポート
    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(g_windowWidth);
    viewport.Height = static_cast<float>(g_windowHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // シザー矩形
    D3D12_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = static_cast<LONG>(g_windowWidth);
    scissorRect.bottom = static_cast<LONG>(g_windowHeight);

    // パイプライン・ルートシグネチャ
    g_commandList->SetPipelineState(g_pipelineState);
    g_commandList->SetGraphicsRootSignature(g_rootSignature);

    // ビューポート・シザー設定
    g_commandList->RSSetViewports(1, &viewport);
    g_commandList->RSSetScissorRects(1, &scissorRect);

    // IA 設定
    g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
    g_commandList->IASetIndexBuffer(&g_indexBufferView);

    // 描画
    g_commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

    // コマンドリストを閉じる
    g_commandList->Close();

    // 実行
    ID3D12CommandList* cmdLists[] = { g_commandList };
    g_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    // Present
    g_swapChain->Present(1, 0);

    // GPU 完了待ち
    WaitForPreviousFrame();
}
