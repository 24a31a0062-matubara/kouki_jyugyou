#include "polygon.h"
#include<cassert>
#include<DirectXMath.h>
#include<d3d12.h>

struct Vertex
{
	DirectX::XMFLOAT3 position;  // 頂点座標 (x, y, z)
	DirectX::XMFLOAT4 color;     // 頂点色 (r, g, b, a)
};

Vertex triangleVertices[] =
{
	{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // 赤
	{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 緑
	{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }  // 青
};

UINT16 triangleIndices[] = { 0, 1, 2 };

int D3D12_resource()
{
	D3D12_HEAP_PROPERTIES heapProperty{};
	heapProperty.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // ←修正
	heapProperty.CreationNodeMask = 1;
	heapProperty.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = sizeof(triangleVertices);
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
	ID3D12Resource* vertexBuffer;
    ID3D12Device* device = nullptr;   
    device->CreateCommittedResource  
    (  
       &heapProperty,  
       D3D12_HEAP_FLAG_NONE,  
       &resourceDesc,  
       D3D12_RESOURCE_STATE_GENERIC_READ,  
       nullptr,  
       IID_PPV_ARGS(&vertexBuffer)  
    );
	return 0;
}



void data()
{
	Vertex* data{};

	VertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data));

	memcpy(data, triangleVertices, sizeof(triangleVertices));

	vertexBuffer->Unmap(0, nullptr);
}
