struct VSInput
{
    float4 position : POSITION;
    float3 normal   : NORMAL;
    float3 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float3 color    : COLOR;
    float3 viewVec  : TEXCOORD1;
    float3 lightVec : TEXCOORD2;
};

cbuffer SceneConstantBuffer : register(b0, space0)
{
    float4x4 projection;
    float4x4 model;
    float4   padding[4];
}

PSInput VSMain( VSInput input )
{
    PSInput output = (PSInput) 0;
    output.normal = input.normal;
    output.color = input.color;
    output.position = mul(projection, mul(model, input.position));

    float4 pos = mul(model, float4(input.position.xyz, 1.0));
    output.normal = mul((float4x3)model, input.normal).xyz;

    float3 lightPos = float3(1.0f, -1.0f, 1.0f);
    output.lightVec = lightPos.xyz - pos.xyz;
    output.viewVec  = -pos.xyz;
    
    return output;
}

float4 PSMain( PSInput input ) : SV_Target
{
    float3 n = normalize(input.normal);
    float3 l = normalize(input.lightVec);
    float3 v = normalize(input.viewVec);
    float3 r = reflect(-l, n);
    float3 ambient = float3(0.1, 0.1, 0.1);
    float3 diffuse = max(dot(n, l), 0.0) * float3(1.0, 1.0, 1.0);
    float3 specular = pow(max(dot(r, v), 0.0), 16.0) * float3(0.75, 0.75, 0.75);
    return float4((ambient + diffuse) * input.color.rgb + specular, 1.0);
}