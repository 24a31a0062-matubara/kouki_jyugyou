#include "polygon.h"
#include <vector>
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

using namespace DirectX;

void polygon::Initialize(
    ID3D12Device* device,
    const std::vector<XMFLOAT3>& positions,
    const std::vector<XMFLOAT4>& colors)
{
    vertexCount = (UINT)positions.size();

    std::vector<Vertex> vertices(vertexCount);
    for (UINT i = 0; i < vertexCount; i++)
    {
        vertices[i].pos = positions[i];
        vertices[i].color = colors[i];
    }

    UINT bufferSize = sizeof(Vertex) * vertexCount;

    // ヒープ設定（UPLOAD）
    D3D12_HEAP_PROPERTIES heapProp{};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc{};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = bufferSize;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    // バッファ作成
    device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)
    );

    // データ転送
    void* mapped = nullptr;
    vertexBuffer->Map(0, nullptr, &mapped);
    memcpy(mapped, vertices.data(), bufferSize);
    vertexBuffer->Unmap(0, nullptr);

    // VBビュー
    vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbView.StrideInBytes = sizeof(Vertex);
    vbView.SizeInBytes = bufferSize;
}

void polygon::Draw(ID3D12GraphicsCommandList* cmdList)
{
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &vbView);
    cmdList->DrawInstanced(vertexCount, 1, 0, 0);
}
