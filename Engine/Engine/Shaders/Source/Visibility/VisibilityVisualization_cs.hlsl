#include "Utility.hlsli"
#include "Common.hlsli"

struct DrawData
{
    uint visualizationType;
};

PUSH_CONSTANT(DrawData, u_visData);

Texture2D<uint2> u_visibilityBuffer;
RWTexture2D<float4> o_output;

static const uint VIS_NONE = 0;
static const uint VIS_OBJECT_ID = 1;
static const uint VIS_TRIANGLE_ID = 2;

[numthreads(16, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    uint2 outputSize;
    o_output.GetDimensions(outputSize.x, outputSize.y);
    
    if (threadId.x >= outputSize.x || threadId.y >= outputSize.y)
    {
        return;
    }
    
    const uint2 visId = u_visibilityBuffer.Load(int3(threadId.xy, 0));
    
    if (visId.x == UINT32_MAX && visId.y == UINT32_MAX)
    {
        return;
    }
    
    if (u_visData.visualizationType == VIS_OBJECT_ID)
    {
        o_output[threadId.xy] = float4(GetRandomColor(visId.x), 1.f);
    }
    else if (u_visData.visualizationType == VIS_TRIANGLE_ID)
    {
        o_output[threadId.xy] = float4(GetRandomColor(visId.y), 1.f);
    }
}