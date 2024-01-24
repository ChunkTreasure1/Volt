#include "Resources.hlsli"

static const float m_pi = 3.14159265359f;

struct Constants
{
    RWTexture<float4> output;
    
    TTexture<float4> equirectangularMap;
    TextureSampler linearSampler;
    
    uint2 textureSize;
};

float3 GetCubeMapTexCoord(uint3 dispatchId, uint2 textureSize)
{
    float2 ST = dispatchId.xy / float2(textureSize.x, textureSize.y);
    float2 UV = 2.f * float2(ST.x, 1.f - ST.y) - 1.f;

    float3 result = 0.f;
    switch (dispatchId.z)
    {
        case 0:
            result = float3(1.f, UV.y, -UV.x);
            break;
        case 1:
            result = float3(-1.f, UV.y, UV.x);
            break;
        case 2:
            result = float3(UV.x, 1.f, -UV.y);
            break;
        case 3:
            result = float3(UV.x, -1.f, UV.y);
            break;
        case 4:
            result = float3(UV.x, UV.y, 1.f);
            break;
        case 5:
            result = float3(-UV.x, UV.y, -1.f);
            break;
    }

    return normalize(result);
}

[numthreads(32, 32, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    float3 cubeTexCoord = GetCubeMapTexCoord(dispatchId, constants.textureSize);

	// Calculate sampling coords for equirectangular texture
	// https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

    float phi = atan2(cubeTexCoord.z, cubeTexCoord.x);
    float theta = acos(cubeTexCoord.y);

    float4 color = constants.equirectangularMap.SampleLevel2D(constants.linearSampler.Get(), float2(phi / (m_pi * 2.f), theta / m_pi), 0);
    constants.output.Store2DArray(dispatchId, color);
}