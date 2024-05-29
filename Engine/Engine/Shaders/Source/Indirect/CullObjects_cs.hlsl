#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Culling.hlsli"

struct Constants
{
    RWTypedBuffer<uint> meshletCount;
    RWTypedBuffer<uint2> meshletToObjectIdAndOffset;
    
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<GPUMesh> meshBuffer;
    
    uint objectCount;
    uint cullingMode;
    
    float frustum0;
    float frustum1;
    float frustum2;
    float frustum3;

    float nearPlane;
    float farPlane;

    float3 orthographicFrustumMin;
    float3 orthographicFrustumMax;
    float4x4 viewMatrix;
};

bool IsInFrustum(in Constants constants, in BoundingSphere sphere)
{
    
    const float3 center = mul(constants.viewMatrix, float4(sphere.center, 1.f)).xyz;

    bool visible = true;
    
    visible = visible && center.z * constants.frustum1 - abs(center.x) * constants.frustum0 > -sphere.radius;
    visible = visible && center.z * constants.frustum3 - abs(center.y) * constants.frustum2 > -sphere.radius;
    visible = visible && center.z + sphere.radius > constants.nearPlane && center.z - sphere.radius < constants.farPlane;
    
    return visible;
}

#define THREAD_GROUP_SIZE 256

[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint threadId : SV_DispatchThreadID, uint groupThreadId : SV_GroupThreadID)
{
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

#if 1
    bool visible = true;

    if (constants.cullingMode == CullingMode::Perspective)
    {
        PerspectiveFrustumInfo perspectiveInfo;
        perspectiveInfo.frustum0 = constants.frustum0;
        perspectiveInfo.frustum1 = constants.frustum1;
        perspectiveInfo.frustum2 = constants.frustum2;
        perspectiveInfo.frustum3 = constants.frustum3;
        perspectiveInfo.nearPlane = constants.nearPlane;
        perspectiveInfo.farPlane = constants.farPlane;
        perspectiveInfo.viewMatrix = constants.viewMatrix;

        visible = IsInFrustum_Perspective(perspectiveInfo, objectDrawData.boundingSphere);
    }
    else if (constants.cullingMode == CullingMode::Orthographic)
    {
        AABB aabb;
        aabb.min = constants.orthographicFrustumMin;
        aabb.max = constants.orthographicFrustumMax;

        visible = IsInFrustum_Orthographic(aabb, objectDrawData.boundingSphere);
    }

#else
    bool visible = IsInFrustum(constants, objectDrawData.boundingSphere);
#endif

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
}