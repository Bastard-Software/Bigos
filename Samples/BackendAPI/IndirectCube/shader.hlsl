struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer SceneConstantBuffer : register(b0, space0)
{
    column_major float4x4 model;
    column_major float4x4 view;
    column_major float4x4 projection;
}

PSInput VSMain(VSInput input)
{
    float4 newPosition = float4(input.position, 1.0f);
    // newPosition = mul(newPosition, model);
    // newPosition = mul(newPosition, view);
    // newPosition = mul(newPosition, projection);

    newPosition = mul( model, newPosition);
    newPosition = mul( view, newPosition);
    newPosition = mul( projection, newPosition);

    PSInput output;
    output.position = newPosition;
    output.uv = input.uv;

    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return float4(input.uv, 0.0f, 1.0f);
}