#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"
#include "Culling.hlsli"

struct Constants
{
    RWTypedBuffer<uint> survivingMeshlets;
    RWTypedBuffer<uint> survivingMeshletCount;
    
    TypedBuffer<uint> meshletCount;
    TypedBuffer<uint2> meshletToObjectIdAndOffset;
    
    TypedBuffer<Meshlet> gpuMeshlets;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    UniformBuffer<ViewData> viewData;
    
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

bool ConeCull(in float3 center, in float radius, float3 coneAxis, float coneCutoff, float3 cameraPosition)
{
    return dot(center - cameraPosition, coneAxis) >= coneCutoff * length(center - cameraPosition) + radius;
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

    const float3 globalScale = float3(length(objectData.transform[0]), length(objectData.transform[1]), length(objectData.transform[2]));
    const float maxScale = max(max(globalScale.x, globalScale.y), globalScale.z);
    BoundingSphere cullingSphere = BoundingSphere::Construct(meshlet.boundingSphereRadius * maxScale, mul(objectData.transform, float4(meshlet.boundingSphereCenter, 1.f)).xyz);    

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

        visible = IsInFrustum_Perspective(perspectiveInfo, cullingSphere);
    }
    else if (constants.cullingMode == CullingMode::Orthographic)
    {
        AABB aabb;
        aabb.min = constants.orthographicFrustumMin;
        aabb.max = constants.orthographicFrustumMax;
        visible = IsInFrustum_Orthographic(aabb, cullingSphere);
    }

    //const ViewData viewData = constants.viewData.Load(0);
    //visible = visible && ConeCull(meshlet.boundingSphereCenter, meshlet.boundingSphereRadius, meshlet.cone.xyz, meshlet.cone.w, cameraData.position.xyz);
    
    if (visible)
    {
        uint offset;
        constants.survivingMeshletCount.InterlockedAdd(0, 1, offset);
        constants.survivingMeshlets.Store(offset, meshletIndex);
    }
}