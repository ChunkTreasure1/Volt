#include "Buffers.hlsli"
#include "Material.hlsli"
#include "SamplerStates.hlsli"
#include "Bindless.hlsli"

static uint CullMode_None = 0;
static uint CullMode_Frustum = 1;
static uint CullMode_CameraFrustum = 2;
static uint CullMode_AABB = 3;

struct DrawCullData
{
    float P00, P11, zNear, zFar;
    
    float frustum0;
    float frustum1;
    float frustum2;
    float frustum3;
    
    uint drawCallCount;
    uint materialFlags;
    uint cullMode;
    uint padding;
    
    float4 aabbMin;
    float4 aabbMax;
};

[[vk::push_constant]] DrawCullData u_cullData;

// 2D Polyhedral Bounds of a Clipped, Perspective-Projected 3D Sphere. Michael Mara, Morgan McGuire. 2013
bool projectSphere(float3 C, float r, float znear, float P00, float P11, out float4 aabb)
{
    if (C.z < r + znear)
        return false;

    float2 cx = -C.xz;
    float2 vx = float2(sqrt(dot(cx, cx) - r * r), r);
    float2 minx = mul(float2x2(vx.x, vx.y, -vx.y, vx.x), cx);
    float2 maxx = mul(float2x2(vx.x, -vx.y, vx.y, vx.x), cx);

    float2 cy = -C.yz;
    float2 vy = float2(sqrt(dot(cy, cy) - r * r), r);
    float2 miny = mul(float2x2(vy.x, vy.y, -vy.y, vy.x), cy);
    float2 maxy = mul(float2x2(vy.x, -vy.y, vy.y, vy.x), cy);

    aabb = float4(minx.x / minx.y * P00, miny.x / miny.y * P11, maxx.x / maxx.y * P00, maxy.x / maxy.y * P11);
    aabb = aabb.xwzy * float4(0.5f, -0.5f, 0.5f, -0.5f) + 0.5f; // clip space -> uv space

    return true;
}

bool IsInFrustum(in ObjectData objectData)
{
    const float3 center = mul(u_cameraData.view, float4(objectData.boundingSphereCenter, 1.f)).xyz;
    const float radius = objectData.boundingSphereRadius;
    
    bool visible = true;
    
    visible = visible && center.z * u_cullData.frustum1 - abs(center.x) * u_cullData.frustum0 > -radius;
    visible = visible && center.z * u_cullData.frustum3 - abs(center.y) * u_cullData.frustum2 > -radius;
    visible = visible && center.z + radius > u_cullData.zNear && center.z - radius < u_cullData.zFar;
    
    return visible;
}

const bool IsInAABB(in ObjectData objectData)
{
    const float3 sphereCenter = objectData.boundingSphereCenter;
    const float sphereRadius = objectData.boundingSphereRadius;

    float3 closestPoint = 0.f;
    
    closestPoint.x = max(u_cullData.aabbMin.x, min(sphereCenter.x, u_cullData.aabbMax.x));
    closestPoint.y = max(u_cullData.aabbMin.y, min(sphereCenter.y, u_cullData.aabbMax.y));
    closestPoint.z = max(u_cullData.aabbMin.z, min(sphereCenter.z, u_cullData.aabbMax.z));

    const float distance = pow((sphereCenter.x - closestPoint.x), 2) + pow((sphereCenter.y - closestPoint.y), 2) + pow((sphereCenter.z - closestPoint.z), 2);
    return distance <= (sphereRadius * sphereRadius);
}

[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    const uint globalId = threadId.x;
    if (globalId < u_cullData.drawCallCount)
    {
        const uint objectId = u_indirectBatches[globalId].objectId;
        const ObjectData objectData = u_objectData[objectId];
        const Material material = GetMaterial(objectData.materialIndex);
        
        const bool hasMaterialFlags = ((u_cullData.materialFlags & material.GetFlags()) != 0) || (u_cullData.materialFlags == 0);
        
        bool visible = true;
        if (u_cullData.cullMode != CullMode_None)
        {
            if (u_cullData.cullMode == CullMode_Frustum || u_cullData.cullMode == CullMode_CameraFrustum)
            {
                visible = IsInFrustum(objectData);
            }
            else if (u_cullData.cullMode == CullMode_AABB)
            {
                visible = IsInAABB(objectData);
            }
        }
        
        if (visible && hasMaterialFlags)
        {
            const uint batchId = u_indirectBatches[globalId].batchId;
            const uint baseIndex = u_indirectBatches[globalId].firstInstance;
        
            const uint countOffset = batchId * 4;
        
            uint drawIndex;
            u_indirectCounts.InterlockedAdd(countOffset, 1, drawIndex);
            u_drawToObjectId[baseIndex + drawIndex] = objectId;
        }
    }
}