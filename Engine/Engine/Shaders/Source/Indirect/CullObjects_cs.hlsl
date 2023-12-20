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

[numthreads(256, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (threadId >= constants.objectCount)
    {
        return;
    }
 
    const ObjectDrawData objectDrawData = constants.objectDrawDataBuffer.Load(threadId);
    
    bool visible = true;
    //IsInFrustum(constants, objectDrawData.boundingSphereCenter, objectDrawData.boundingSphereRadius);
    
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
}