#include "Common.hlsli"
#include "Defines.hlsli"
#include "SamplerStates.hlsli"
#include "Utility.hlsli"

struct PushConstants
{
    float2 texelSize;
    float2 dir;
    float sssWidth;
};

[[vk::push_constant]] PushConstants u_pushConstants;

Texture2D u_diffuseTexture : register(t0, SPACE_OTHER);
Texture2D u_depthTexture : register(t1, SPACE_OTHER);
RWTexture2D<float4> u_outputTexture : register(u2, SPACE_OTHER);

static const float4 m_kernel[] =
{
    float4(0.530605, 0.613514, 0.739601, 0),
	float4(0.000973794, 1.11862e-005, 9.43437e-007, -3),
	float4(0.00333804, 7.85443e-005, 1.2945e-005, -2.52083),
	float4(0.00500364, 0.00020094, 5.28848e-005, -2.08333),
	float4(0.00700976, 0.00049366, 0.000151938, -1.6875),
	float4(0.0094389, 0.00139119, 0.000416598, -1.33333),
	float4(0.0128496, 0.00356329, 0.00132016, -1.02083),
	float4(0.017924, 0.00711691, 0.00347194, -0.75),
	float4(0.0263642, 0.0119715, 0.00684598, -0.520833),
	float4(0.0410172, 0.0199899, 0.0118481, -0.333333),
	float4(0.0493588, 0.0367726, 0.0219485, -0.1875),
	float4(0.0402784, 0.0657244, 0.04631, -0.0833333),
	float4(0.0211412, 0.0459286, 0.0378196, -0.0208333),
	float4(0.0211412, 0.0459286, 0.0378196, 0.0208333),
	float4(0.0402784, 0.0657244, 0.04631, 0.0833333),
	float4(0.0493588, 0.0367726, 0.0219485, 0.1875),
	float4(0.0410172, 0.0199899, 0.0118481, 0.333333),
	float4(0.0263642, 0.0119715, 0.00684598, 0.520833),
	float4(0.017924, 0.00711691, 0.00347194, 0.75),
	float4(0.0128496, 0.00356329, 0.00132016, 1.02083),
	float4(0.0094389, 0.00139119, 0.000416598, 1.33333),
	float4(0.00700976, 0.00049366, 0.000151938, 1.6875),
	float4(0.00500364, 0.00020094, 5.28848e-005, 2.08333),
	float4(0.00333804, 7.85443e-005, 1.2945e-005, 2.52083),
	float4(0.000973794, 1.11862e-005, 9.43437e-007, 3),
};

[numthreads(16, 16, 1)]
void main(uint2 pixelCoords : SV_DispatchThreadID)
{
    const float4 diffuse = u_diffuseTexture.Load(int3(pixelCoords, 0));

	if (diffuse.a == 0.f)
    {
        u_outputTexture[pixelCoords] = diffuse;
        return;
    }
	
    const float depth = LinearizeDepth(u_depthTexture.Load(int3(pixelCoords, 0)).r);
    const float rayRadiusUV = 0.5f * u_pushConstants.sssWidth / depth;
	
    if (rayRadiusUV <= u_pushConstants.texelSize.x)
    {
        u_outputTexture[pixelCoords] = diffuse;
        return;
    }
	
    float2 finalStep = rayRadiusUV * u_pushConstants.dir;
    finalStep *= diffuse.a;
    finalStep *= 1.f / 3.f; // divide by 3 as the kernels range from -3 to 3
	
	// accumulate the center sample:
    float4 colorBlurred = diffuse;
    colorBlurred.rgb *= m_kernel[0].rgb;
	
    const float2 texCoords = (float2(pixelCoords) + 0.5f) * u_pushConstants.texelSize;

    for (int i = 1; i < 25; i++)
    {
        float2 offset = texCoords + m_kernel[i].a * finalStep;
        float4 color = u_diffuseTexture.SampleLevel(u_linearSampler, offset, 0.f);
        float offsetDepth = LinearizeDepth(u_depthTexture.SampleLevel(u_pointSampler, offset, 0.f).r);
		
        const float maxDepthDiff = 0.01f;
        float alpha = min(distance(depth, offsetDepth) / maxDepthDiff, maxDepthDiff);

        alpha *= 1.f - color.a;
        color.rgb = lerp(color.rgb, diffuse.rgb, alpha);
		
        colorBlurred.rgb += m_kernel[i].rgb * color.rgb;
    }
	
    u_outputTexture[pixelCoords] = colorBlurred;
}