RWTexture2DArray<float3> o_output;
Texture2D<float4> u_equirectangularMap;

SamplerState u_linearSampler;

static const float m_pi = 3.14159265359f;

float3 GetCubeMapTexCoord(uint3 dispatchId)
{
    uint2 texSize;
    uint elements;

    o_output.GetDimensions(texSize.x, texSize.y, elements);

    float2 ST = dispatchId.xy / float2(texSize.x, texSize.y);
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
    float3 cubeTexCoord = GetCubeMapTexCoord(dispatchId);

	// Calculate sampling coords for equirectangular texture
	// https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

    float phi = atan2(cubeTexCoord.z, cubeTexCoord.x);
    float theta = acos(cubeTexCoord.y);

    float4 color = u_equirectangularMap.SampleLevel(u_linearSampler, float2(phi / (m_pi * 2.f), theta / m_pi), 0);
    o_output[dispatchId] = color.rgb;
}