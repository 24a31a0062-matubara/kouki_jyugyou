#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>

class polygon
{
public:
    polygon() = default;

    void Initialize(
        ID3D12Device* device,
        const std::vector<DirectX::XMFLOAT3>& positions,
        const std::vector<DirectX::XMFLOAT4>& colors);

    void Draw(ID3D12GraphicsCommandList* cmdList);

private:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vbView{};
    UINT vertexCount = 0;
};
