struct VSInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D t_texture : register(t0, space0);
SamplerState s_sampler : register(s1, space1);

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = input.position;
    output.uv = input.uv;

    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return t_texture.Sample(s_sampler, input.uv);
}