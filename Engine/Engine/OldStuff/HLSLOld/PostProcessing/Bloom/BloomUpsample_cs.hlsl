// From https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

#include "SamplerStates.hlsli"

struct PushConstants
{
    float filterRadius;
};

[[vk::push_constant]] PushConstants u_pushConstants;

RWTexture2D<float4> o_resultTexture : register(u0, SPACE_OTHER);
Texture2D<float4> u_colorSource : register(t1, SPACE_OTHER);

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 groupId : SV_GroupID)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);

    const uint2 groupThreadId = remap_lane_8x8(groupIndex);
    const uint2 pixelCoords = groupId.xy * 8 + groupThreadId;
    
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    const float2 texCoords = float2((float) pixelCoords.x / (float) (outputWidth - 1), (float) pixelCoords.y / (float) (outputHeight - 1));
    
    // The filter kernel is applied with a radius, specified in texture
	// coordinates, so that the radius will vary across mip resolutions.
    const float x = u_pushConstants.filterRadius;
    const float y = u_pushConstants.filterRadius;

	// Take 9 samples around current texel:
	// a - b - c
	// d - e - f
	// g - h - i
	// === ('e' is the current texel) ===
    const float3 a = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - x, texCoords.y + y), 0.f).rgb;
    const float3 b = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x, texCoords.y + y), 0.f).rgb;
    const float3 c = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + x, texCoords.y + y), 0.f).rgb;

    const float3 d = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - x, texCoords.y), 0.f).rgb;
    const float3 e = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x, texCoords.y), 0.f).rgb;
    const float3 f = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + x, texCoords.y), 0.f).rgb;

    const float3 g = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - x, texCoords.y - y), 0.f).rgb;
    const float3 h = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x, texCoords.y - y), 0.f).rgb;
    const float3 i = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + x, texCoords.y - y), 0.f).rgb;

	// Apply weighted distribution, by using a 3x3 tent filter:
	//  1   | 1 2 1 |
	// -- * | 2 4 2 |
	// 16   | 1 2 1 |

    float3 result = e * 4.f;
    result += (b + d + f + h) * 2.f;
    result += (a + c + g + i);
    result *= 1.f / 16.f;
    
    o_resultTexture[pixelCoords] = float4(result, 1.f);
}