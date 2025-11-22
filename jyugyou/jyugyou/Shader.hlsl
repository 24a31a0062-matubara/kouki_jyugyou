// Shader.hlsl

// 頂点シェーダー入力
struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

// 頂点シェーダー出力 → ピクセルシェーダー入力
struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// 頂点シェーダー
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = float4(input.position, 1.0f);
    output.color = input.color;
    return output;
}

// ピクセルシェーダー
float4 PSMain(VSOutput input) : SV_TARGET
{
    return input.color;
}
