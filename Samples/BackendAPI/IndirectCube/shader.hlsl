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

Texture2D t_texture : register(t1, space0);
SamplerState s_sampler : register(s0, space1);

PSInput VSMain(VSInput input)
{
    float4 newPosition = float4(input.position, 1.0f);
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
    return t_texture.Sample(s_sampler, input.uv);
}