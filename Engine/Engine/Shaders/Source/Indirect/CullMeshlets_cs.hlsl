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
    UniformBuffer<ViewData> viewData;
    
    float frustum0;
    float frustum1;
    float frustum2;
    float frustum3;
};

bool IsInFrustum(in Constants constants, in float4x4 transform, in float3 boundingSphereCenter, in float boundingSphereRadius)
{
    const ViewData viewData = constants.viewData.Load();
    
    const float3 globalScale = float3(length(transform[0]), length(transform[1]), length(transform[2]));
    const float maxScale = max(max(globalScale.x, globalScale.y), globalScale.z);
    boundingSphereRadius *= maxScale;
    
    const float3 center = mul(viewData.view, mul(transform, float4(boundingSphereCenter, 1.f))).xyz;
    
    bool visible = true;
    
    visible = visible && center.z * constants.frustum1 - abs(center.x) * constants.frustum0 > -boundingSphereRadius;
    visible = visible && center.z * constants.frustum3 - abs(center.y) * constants.frustum2 > -boundingSphereRadius;
    visible = visible && center.z + boundingSphereRadius > viewData.nearPlane && center.z - boundingSphereRadius < viewData.farPlane;
    
    return true;
}

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
    
    bool visible = IsInFrustum(constants, objectData.transform, meshlet.boundingSphereCenter, meshlet.boundingSphereRadius);
    
    //const ViewData viewData = constants.viewData.Load(0);
    //visible = visible && ConeCull(meshlet.boundingSphereCenter, meshlet.boundingSphereRadius, meshlet.cone.xyz, meshlet.cone.w, cameraData.position.xyz);
    
    if (visible)
    {
        uint offset;
        constants.survivingMeshletCount.InterlockedAdd(0, 1, offset);
        constants.survivingMeshlets.Store(offset, meshletIndex);
    }
}