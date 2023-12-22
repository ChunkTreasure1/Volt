#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

struct Constants
{
    RWTypedBuffer<uint> survivingMeshlets;
    RWTypedBuffer<uint> survivingMeshletCount;
    
    TypedBuffer<uint> meshletCount;
    TypedBuffer<uint2> meshletToObjectIdAndOffset;
    
    TypedBuffer<Meshlet> gpuMeshlets;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<CameraData> cameraData;
    
    float frustum0;
    float frustum1;
    float frustum2;
    float frustum3;
};

bool IsInFrustum(in Constants constants, in float4x4 transform, in float3 boundingSphereCenter, in float boundingSphereRadius)
{
    const CameraData cameraData = constants.cameraData.Load(0);
    
    const float3 globalScale = float3(length(transform[0]), length(transform[1]), length(transform[2]));
    const float maxScale = max(max(globalScale.x, globalScale.y), globalScale.z);
    boundingSphereRadius *= maxScale;
    
    const float3 center = mul(cameraData.view, mul(transform, float4(boundingSphereCenter, 1.f))).xyz;
    
    bool visible = true;
    
    visible = visible && center.z * constants.frustum1 - abs(center.x) * constants.frustum0 > -boundingSphereRadius;
    visible = visible && center.z * constants.frustum3 - abs(center.y) * constants.frustum2 > -boundingSphereRadius;
    visible = visible && center.z + boundingSphereRadius > cameraData.farPlane && center.z - boundingSphereRadius < cameraData.nearPlane;
    
    return visible;
}

[numthreads(256, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (threadId >= constants.meshletCount.Load(0))
    {
        return;
    }
 
    const uint2 objectIdAndOffset = constants.meshletToObjectIdAndOffset.Load(threadId);
    const ObjectDrawData objectData = constants.objectDrawDataBuffer.Load(objectIdAndOffset.x);
    
    const uint meshletIndex = objectData.meshletStartOffset + threadId - objectIdAndOffset.y;
    const Meshlet meshlet = constants.gpuMeshlets.Load(meshletIndex);
    
    bool visible = true; /*IsInFrustum(constants, objectData.transform, meshlet.boundingSphereCenter, meshlet.boundingSphereRadius);*/
    
    if (visible)
    {
        uint offset;
        constants.survivingMeshletCount.InterlockedAdd(0, 1, offset);
        constants.survivingMeshlets.Store(offset, meshletIndex);
    }
}