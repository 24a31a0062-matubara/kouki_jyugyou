#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>

class polygon
{
public:
    polygon();
    void Initialize(ID3D12Device* device);
    void Draw(ID3D12GraphicsCommandList* cmdList);

private:
    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vbView{};
};
