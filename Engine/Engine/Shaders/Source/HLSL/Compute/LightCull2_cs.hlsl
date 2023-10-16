struct LightCullData
{
    uint2 tileCount;
    uint2 targetSize;
};

[[vk::push_constant]] LightCullData u_cullData;

[numthreads(32, 1, 1)]
void main(uint2 location : SV_DispatchThreadID, uint localInvocationIndex : SV_GroupIndex, uint2 workGroupID : SV_GroupID)
{
    int2 tileId = int2(workGroupID);
    uint tileIndex = tileId.y * u_cullData.tileCount.x + tileId.x;
    
    if (localInvocationIndex == 0)
    {
               
    }
}