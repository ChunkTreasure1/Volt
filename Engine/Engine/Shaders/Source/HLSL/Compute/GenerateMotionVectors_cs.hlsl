#include "Defines.hlsli"
#include "Utility.hlsli"

Texture2D<float> u_sourceDepth : register(t0, SPACE_OTHER);

RWTexture2D<float4> o_motionVectors : register(u1, SPACE_OTHER);
RWTexture2D<float4> o_depth : register(u2, SPACE_OTHER);

struct PushConstants
{
    float4x4 reprojectionMatrix;
    float2 viewportSize2x;
    float2 jitter;
};

[[vk::push_constant]] PushConstants u_pushConstants;

// from [0, width], [0, height] to [-1, 1], [-1, 1]
float2 ClipSpaceToNDCPositionXY(float2 svPos)
{
    return svPos * float2(u_pushConstants.viewportSize2x.x, -u_pushConstants.viewportSize2x.y) + float2(-1.0f, 1.0f);
}

float2 NDCToClipSpacePositionXY(float2 ndcPos)
{
    return (ndcPos - float2(-1.0f, 1.0f)) / float2(u_pushConstants.viewportSize2x.x, -u_pushConstants.viewportSize2x.y);
}

float ViewspaceDepthToTAACompDepthFunction(float viewspaceDepth)
{
    // better utilizes FP16 precision
    return viewspaceDepth / 100.0;
}

float2 CalcVelocity(float2 newPos, float2 oldPos)
{
    oldPos.xy = (oldPos.xy + 1) / 2.0f;
    oldPos.y = 1 - oldPos.y;
    
    newPos.xy = (newPos.xy + 1) / 2.0f;
    newPos.y = 1 - newPos.y;
    
    return (newPos - oldPos).xy;
}

[numthreads(8, 8, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixCoord = dispatchThreadID;
    
    float depthNDC = u_sourceDepth.Load(int3(pixCoord, 0)).x;
    float depth = LinearizeDepth(depthNDC);

    float2 clipXY = pixCoord + 0.5f;
    float2 ndcXY = ClipSpaceToNDCPositionXY(clipXY);
    
    float4 reprojectedPos = mul(u_pushConstants.reprojectionMatrix, float4(ndcXY.xy, depthNDC, 1.f));
    reprojectedPos.xyz /= reprojectedPos.w;
    
    float reprojectedDepth = LinearizeDepth(reprojectedPos.z);
    
    depth = ViewspaceDepthToTAACompDepthFunction(depth);
    reprojectedDepth = ViewspaceDepthToTAACompDepthFunction(reprojectedDepth);
    
    reprojectedDepth *= 0.999f;
    
    float3 delta;
    delta.xy = NDCToClipSpacePositionXY(reprojectedPos.xy) - clipXY;
    delta.z = reprojectedDepth - depth;
    
    delta.xy -= u_pushConstants.jitter;
    
    o_depth[pixCoord] = depth;
    o_motionVectors[pixCoord] = float4(delta.xyz, 0.f);
}