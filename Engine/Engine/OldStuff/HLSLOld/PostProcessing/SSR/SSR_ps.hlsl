#include "Vertex.hlsli"
#include "SamplerStates.hlsli"

#include "Utility.hlsli"

Texture2D<float4> u_depthTexture : register(t0, SPACE_MATERIAL);
Texture2D<float4> u_viewNormalRoughnessTexture : register(t1, SPACE_MATERIAL);
Texture2D<float4> u_sceneColor : register(t2, SPACE_MATERIAL);

static const float RAY_STEP = 0.1f;
static const float MIN_RAY_STEP = 0.1f;
static const int MAX_STEPS = 30;
static const float SEARCH_DIST = 5.f;
static const int NUM_BINARY_SEARCH_STEPS = 5;
static const float REFLECTION_SPECULAR_FALLOFF_EXPONENT = 3.f;

float3 BinarySearch(inout float3 dir, inout float3 hitCoord, inout float dDepth)
{
    float depth;
    float4 projectedCoords;
    
    for (int i = 0; i < NUM_BINARY_SEARCH_STEPS; i++)
    {
        projectedCoords = mul(u_cameraData.projection, float4(hitCoord, 1.f));
        projectedCoords.xy /= projectedCoords.w;
        projectedCoords.xy = float2(projectedCoords.x * 0.5f + 0.5f, -projectedCoords.y * 0.5f + 0.5f);
        
        float d = u_depthTexture.Sample(u_linearSampler, projectedCoords.xy).r;
        depth = ReconstructViewPosition(projectedCoords.xy, d).z;
        
        dDepth = hitCoord.z - depth;
        
        dir *= 0.5f;
        if (dDepth > 0.f)
        {
            hitCoord += dir;
        }
        else
        {
            hitCoord -= dir;
        }
    }
    
    projectedCoords = mul(u_cameraData.projection, float4(hitCoord, 1.f));
    projectedCoords.xy /= projectedCoords.w;
    projectedCoords.xy = float2(projectedCoords.x * 0.5f + 0.5f, -projectedCoords.y * 0.5f + 0.5f);
    
    return float3(projectedCoords.xy, depth);
}

float4 RayMarch(in float3 dir, inout float3 hitCoord, out float dDepth)
{
    dir *= RAY_STEP;
    
    float depth;
    float4 projectedCoords;
    
    for (int i = 0; i < MAX_STEPS; i++)
    {
        hitCoord += dir;
        
        projectedCoords = mul(u_cameraData.projection, float4(hitCoord, 1.f));
        projectedCoords.xy /= projectedCoords.w;
        projectedCoords.xy = float2(projectedCoords.x * 0.5f + 0.5f, -projectedCoords.y * 0.5f + 0.5f);
        
        float d = u_depthTexture.Sample(u_linearSampler, projectedCoords.xy).r;
        depth = ReconstructViewPosition(projectedCoords.xy, d).z;

        if (depth > 10000)
        {
            continue;
        }
        
        dDepth = hitCoord.z - depth;
        
        if ((dir.z - dDepth) < 1.2f)
        {
            if (dDepth <= 0.f)
            {
                float4 result = float4(BinarySearch(dir, hitCoord, dDepth), 1.f);
                return result;
            }
        }
    }
 
    return float4(projectedCoords.xy, depth, 0.f);
}

float4 main(DefaultFullscreenTriangleVertex input) : SV_Target0
{
    float4 normalRoughness = u_viewNormalRoughnessTexture.Sample(u_pointSampler, input.texCoords);
    float3 viewNormal = normalRoughness.xyz;
    float roughness = normalRoughness.w;
    
    float pixelDepth = u_depthTexture.Sample(u_pointSampler, input.texCoords).r;
    float3 viewPosition = ReconstructViewPosition(input.texCoords, pixelDepth);
    
    const float3 reflected = normalize(reflect(normalize(viewPosition), normalize(viewNormal)));

    float3 hitPos = viewPosition;
    float dDepth;
    
    float4 coords = RayMarch(reflected * max(MIN_RAY_STEP, -viewPosition.z), hitPos, dDepth);
    
    float2 dCoords = smoothstep(0.2f, 0.6f, abs(0.5f - coords.xy));
    
    float screenEdgeFactor = clamp(1.f - (dCoords.x + dCoords.y), 0.f, 1.f);
    float multiplier = pow(1.f - roughness, REFLECTION_SPECULAR_FALLOFF_EXPONENT) * screenEdgeFactor * -reflected.z;
    
    float3 ssr = u_sceneColor.Sample(u_pointSampler, float2(coords.x, coords.y)).rgb * clamp(multiplier, 0.f, 0.9f);
    
    return float4(ssr, roughness);
}