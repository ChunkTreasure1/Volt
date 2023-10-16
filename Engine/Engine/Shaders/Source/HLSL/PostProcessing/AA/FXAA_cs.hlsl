#include "Common.hlsli"
#include "Defines.hlsli"
#include "ComputeUtility.hlsli"

#include "SamplerStates.hlsli"
#include "PostProcessingBase.hlsli"

#define ITERATIONS 12
#define QUALITY(x) 4.0 * x
#define SUBPIXEL_QUALITY 1.0
float RGBToLuminance(float3 col)
{
    return sqrt(dot(col, float3(0.299, 0.587, 0.114)));
}

struct PushConstants
{
    float2 inverseTargetSize;
};

[[vk::push_constant]] PushConstants u_pushConstants;

Texture2D<float4> u_colorSource : register(t1, SPACE_OTHER);

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 groupId : SV_GroupID)
{
    float outputWidth, outputHeight;
    o_resultTexture.GetDimensions(outputWidth, outputHeight);

    const uint2 pixelCoords = RemapThreadIDToPixel(groupIndex, groupId.xy);
    
    if (pixelCoords.x > uint(outputWidth) || pixelCoords.y > uint(outputHeight))
    {
        return;
    }
    
    const float2 texCoords = GetUV(pixelCoords);
    
    float4 color = u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0.f);
    const float lumaCenter = RGBToLuminance(color.rgb);
    
    float2 inverseScreenSize = u_pushConstants.inverseTargetSize;
    
    float lumaDown = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(0, -1)).rgb);
    float lumaUp = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(0, 1)).rgb);
    float lumaLeft = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(-1, 0)).rgb);
    float lumaRight = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(1, 0)).rgb);
    
    float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));
    
    float lumaRange = lumaMax - lumaMin;
    if (lumaRange < max(0.0312, lumaMax * 0.125))
    {
        WriteColor(color, pixelCoords);
        return;
    }
    
    float lumaDownLeft = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(-1, -1)).rgb);
    float lumaUpRight = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(1, 1)).rgb);
    float lumaUpLeft = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(-1, 1)).rgb);
    float lumaDownRight = RGBToLuminance(u_colorSource.SampleLevel(u_linearSamplerClamp, texCoords, 0, int2(1, -1)).rgb);
    
    
    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;
    
    float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaDownRight + lumaUpRight;
    float lumaUpCorners = lumaUpRight + lumaUpLeft;
    
    float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaDown + lumaDownCorners);
    
    bool isHorizontal = (edgeHorizontal >= edgeVertical);
    
    float luma1 = isHorizontal ? lumaDown : lumaLeft;
    float luma2 = isHorizontal ? lumaUp : lumaRight;
    
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;
    
    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
    
    float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
    
    float lumaLocalAverage = 0.0f;
    
    if (is1Steepest)
    {
    // Switch the direction
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    }
    else
    {
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
    }
    
    float2 currentUv = texCoords;
    
    if (isHorizontal)
    {
        currentUv.y -= stepLength * 0.5;
    }
    else
    {
        currentUv.x += stepLength * 0.5;
    }
    
    float2 offset = isHorizontal ? float2(inverseScreenSize.x, 0.0) : float2(0.0, inverseScreenSize.y);
    
    float2 uv1 = currentUv - offset;
    float2 uv2 = currentUv + offset;
    
    float lumaEnd1 = RGBToLuminance(u_colorSource.SampleLevel(u_linearSampler, uv1, 0.f).rgb);
    float lumaEnd2 = RGBToLuminance(u_colorSource.SampleLevel(u_linearSampler, uv2, 0.f).rgb);
    
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;
    
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    
    bool reachedBoth = reached1 && reached2;
    
    if (!reached1)
    {
        uv1 -= offset;
    }
    else
    {
        uv2 += offset;
    }
    
    if (!reachedBoth)
    {

        [unroll]
        for (int i = 2; i < ITERATIONS; i++)
        {
            if (!reached1)
            {
                lumaEnd1 = RGBToLuminance(u_colorSource.SampleLevel(u_linearSampler, uv1, 0.f).rgb);
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
            }
            if (!reached2)
            {
                lumaEnd2 = RGBToLuminance(u_colorSource.SampleLevel(u_linearSampler, uv2, 0.f).rgb);
                lumaEnd2 = lumaEnd2 - lumaLocalAverage;
            }
            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            if (!reached1)
            {
                uv1 -= offset * QUALITY(i);
            }
            if (!reached2)
            {
                uv2 += offset * QUALITY(i);
            }

            if (reachedBoth)
            {
                break;
            }
        }
    }
    float distance1 = isHorizontal ? (texCoords.x - uv1.x) : (texCoords.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - texCoords.x) : (uv2.y - texCoords.y);

// In which direction is the extremity of the edge closer ?
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    float edgeThickness = (distance1 + distance2);

    float pixelOffset = -distanceFinal / edgeThickness + 0.5;
    
        
    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;


    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

    float finalOffset = correctVariation ? pixelOffset : 0.0;
        
    float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

    finalOffset = max(finalOffset, subPixelOffsetFinal);
    float2 finalUv = texCoords;
    if (isHorizontal)
    {
        finalUv.y += finalOffset * stepLength;
    }
    else
    {
        finalUv.x += finalOffset * stepLength;
    }

    // Read the color at the new UV coordinates, and use it.
    float3 finalColor = u_colorSource.SampleLevel(u_linearSampler, finalUv, 0.f).rgb;
    WriteColor(float4(finalColor, 1.f), pixelCoords);
}