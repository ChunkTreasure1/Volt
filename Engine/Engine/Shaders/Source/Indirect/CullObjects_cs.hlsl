#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

struct Constants
{
    RWTypedBuffer<uint> meshletCount;
    RWTypedBuffer<uint2> meshletToObjectIdAndOffset;
    
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer <GPUMesh>meshBuffer;
    TypedBuffer<CameraData> cameraData;
    
    uint objectCount;
    
    float frustum0;
    float frustum1;
    float frustum2;
    float frustum3;
};

bool IsInFrustum(in Constants constants, in float3 boundingSphereCenter, in float boundingSphereRadius)
{
    const CameraData cameraData = constants.cameraData.Load(0);
    
    const float3 center = mul(cameraData.view, float4(boundingSphereCenter, 1.f)).xyz;

    bool visible = true;
    
    visible = visible && center.z * constants.frustum1 - abs(center.x) * constants.frustum0 > -boundingSphereRadius;
    visible = visible && center.z * constants.frustum3 - abs(center.y) * constants.frustum2 > -boundingSphereRadius;
    //visible = visible && center.z + boundingSphereRadius > cameraData.farPlane && center.z - boundingSphereRadius < cameraData.nearPlane;
    
    return visible;
}

#define THREAD_GROUP_SIZE 256

[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint threadId : SV_DispatchThreadID, uint groupThreadId : SV_GroupThreadID)
{
#if 0
    const Constants constants = GetConstants<Constants>();
    
    if (threadId >= constants.objectCount)
    {
        return;
    }
 
    const ObjectDrawData objectDrawData = constants.objectDrawDataBuffer.Load(threadId);
    
    bool visible = IsInFrustum(constants, objectDrawData.boundingSphereCenter, objectDrawData.boundingSphereRadius);
    
    if (visible)
    {
        const GPUMesh mesh = constants.meshBuffer.Load(objectDrawData.meshId);
        
        uint meshletOffset;
        constants.meshletCount.InterlockedAdd(0, mesh.meshletCount, meshletOffset);
        
        for (uint i = 0; i < mesh.meshletCount; ++i)
        {
            constants.meshletToObjectIdAndOffset.Store(meshletOffset + i, uint2(threadId, meshletOffset));
        }
    }
    
#else
    const Constants constants = GetConstants<Constants>();
    
    const uint waveSize = WaveGetLaneCount();
    const uint waveIndex = groupThreadId / THREAD_GROUP_SIZE;
    const uint laneIndex = WaveGetLaneIndex();
    
    if (threadId >= constants.objectCount)
    {
        return;
    }
 
    const ObjectDrawData objectDrawData = constants.objectDrawDataBuffer.Load(threadId);
    const GPUMesh mesh = constants.meshBuffer.Load(objectDrawData.meshId);
    
    bool visible = IsInFrustum(constants, objectDrawData.boundingSphereCenter, objectDrawData.boundingSphereRadius);
    uint meshletCount = visible ? mesh.meshletCount : 0;
    uint totalCount = WaveActiveSum(meshletCount);
    
    uint meshletOffset;
    if (WaveIsFirstLane())
    {
        constants.meshletCount.InterlockedAdd(0, totalCount, meshletOffset);
    }
    
    meshletOffset = WaveReadLaneFirst(meshletOffset);
    uint laneOffset = meshletOffset + WavePrefixSum(meshletCount);
    
    if (visible)
    {
        for (uint i = 0; i < meshletCount; ++i)
        {
            constants.meshletToObjectIdAndOffset.Store(laneOffset + i, uint2(threadId, laneOffset));
        }
    }
#endif
}