// From https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

#include "Common.hlsli"
#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

#include "SamplerStates.hlsli"

RWTexture2D<float4> o_resultTexture : register(u0, SPACE_OTHER);
Texture2D<float4> u_colorSource : register(t1, SPACE_OTHER);

[numthreads(8, 8, 1)]
void main(uint2 threadId : SV_DispatchThreadID)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);

    const uint2 pixelCoords = threadId;
    
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    float2 srcTexelSize = 1.f / float2(outputWidth, outputHeight);
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;
    
    const float2 texCoords = float2((float) pixelCoords.x / (float) (outputWidth - 1), (float) pixelCoords.y / (float) (outputHeight - 1));
    
    const float3 a = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - 2.f * x, texCoords.y + 2.f * y), 0).rgb;
    const float3 b = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x, texCoords.y + 2.f * y), 0).rgb;
    const float3 c = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + 2.f * x, texCoords.y + 2.f * y), 0).rgb;

    const float3 d = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - 2.f * x, texCoords.y), 0).rgb;
    const float3 e = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x, texCoords.y), 0).rgb;
    const float3 f = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + 2.f * x, texCoords.y), 0).rgb;

    const float3 g = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - 2.f * x, texCoords.y - 2.f * y), 0).rgb;
    const float3 h = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x, texCoords.y - 2.f * y), 0).rgb;
    const float3 i = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + 2.f * x, texCoords.y - 2.f * y), 0).rgb;

    const float3 j = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - x, texCoords.y + y), 0).rgb;
    const float3 k = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + x, texCoords.y + y), 0).rgb;
    const float3 l = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x - x, texCoords.y - y), 0).rgb;
    const float3 m = u_colorSource.SampleLevel(u_linearSamplerClamp, float2(texCoords.x + x, texCoords.y - y), 0).rgb;

	// Apply weighted distribution:
	// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
	// a,b,d,e * 0.125
	// b,c,e,f * 0.125
	// d,e,g,h * 0.125
	// e,f,h,i * 0.125
	// j,k,l,m * 0.5
	// This shows 5 square areas that are being sampled. But some of them overlap,
	// so to have an energy preserving downsample we need to make some adjustments.
	// The weights are the distributed, so that the sum of j,k,l,m (e.g.)
	// contribute 0.5 to the final color output. The code below is written
	// to effectively yield this sum. We get:
	// 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    float3 result = e * 0.125f;
    result += (a + c + g + i) * 0.03125f;
    result += (b + d + f + h) * 0.0625f;
    result += (j + k + l + m) * 0.125;

    result = max(result, 0.0001f);
    
    o_resultTexture[pixelCoords] = float4(result, 1.f);
}