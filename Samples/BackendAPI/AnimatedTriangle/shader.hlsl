struct VSInput
{
    float4 position : POSITION;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

cbuffer SceneConstantBuffer : register(b0, space0)
{
    float4 offset;
    float4 padding[15]; // Constant buffer needs to be 256 aligned
}

PSInput VSMain( VSInput input )
{
    PSInput output;
    output.position = input.position + offset;
    output.color    = input.color;

    return output;
}

float4 PSMain( PSInput input ) : SV_Target
{
    return input.color;
}