static const float4 positions[ 3 ] = {
    float4( -0.5f, -0.5f, 0.0f, 1.0f ),
    float4(  0.5f, -0.5f, 0.1f, 1.0f ),
    float4(  0.0f,  0.5f, 0.0f, 1.0f ),
};

float4 VSMain( uint vertexID : SV_VertexID ) : SV_Position
{
    return positions[ vertexID ];
}

float4 PSMain( float4 position : SV_Position ) : SV_Target
{
    return float4( 255.0f / 255.0f, 240.0f / 255.0f, 0.0f / 255.0f, 1.0f );
}