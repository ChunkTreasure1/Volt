#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Structures.hlsli"

#include "MeshShaderConfig.hlsli"
#include "Utility.hlsli"

#include "Atomics.hlsli"

namespace CullingType
{
    static const uint Perspective = 0;
    static const uint Orthographic = 1;
}

struct Constants
{
    vt::RWTypedBuffer<uint> countBuffer;
    vt::RWTypedBuffer<MeshTaskCommand> taskCommands;

    GPUScene gpuScene;

    float4x4 viewMatrix;
    float4 cullingFrustum;
    float nearPlane;
    float farPlane;

    uint cullingType;
};

[numthreads(64, 1, 1)]
void MainCS(uint dispatchThreadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    const uint validPrimitiveDrawDataCount = constants.gpuScene.validPrimitiveDrawDatasBuffer.Load(0);

    if (dispatchThreadId >= validPrimitiveDrawDataCount)
    {
        return;
    }

    // We must offset by one as the first element is the counter.
    const uint primitiveDrawDataIndex = constants.gpuScene.validPrimitiveDrawDatasBuffer.Load(dispatchThreadId + 1);
    const PrimitiveDrawData drawData = constants.gpuScene.primitiveDrawDataBuffer.Load(primitiveDrawDataIndex);
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);

    const float3 center = mul(constants.viewMatrix, float4(drawData.transform.GetWorldPosition(mesh.boundingSphere.center), 1.f)).xyz;
    const float radius = mesh.boundingSphere.radius * max(drawData.transform.scale.x, max(drawData.transform.scale.y, drawData.transform.scale.z));

    bool visible = true;

    if (constants.cullingType == CullingType::Perspective)
    {
        visible = visible && center.z * constants.cullingFrustum.y - abs(center.x) * constants.cullingFrustum.x > -radius;
        visible = visible && center.z * constants.cullingFrustum.w - abs(center.y) * constants.cullingFrustum.z > -radius;
        visible = visible && center.z + radius > constants.nearPlane && center.z - radius < constants.farPlane;
    }
    else if (constants.cullingType == CullingType::Orthographic)
    {
        //float closestX = clamp(center.x, constants.cullingFrustum.x, constants.cullingFrustum.y);
        //float closestY = clamp(center.y, constants.cullingFrustum.z, constants.cullingFrustum.w);
        //
        //float distanceX = center.x - closestX;
        //float distanceY = center.y - closestY;
        //
        //float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
        //visible = visible && distanceSquared < (radius * radius);
    } 
    
    if (visible)
    {
        uint taskGroups = DivideRoundUp(mesh.meshletCount, NUM_AS_THREADS);
        
        uint drawOffset;
        vt::InterlockedAdd(constants.countBuffer, 0, taskGroups, drawOffset);

        // Skip draw calls if AS group count limit is reached. This equals ~4M visible draws or ~32B visible triangles.
        if (drawOffset + taskGroups <= NUM_MAX_AS_GROUP_COUNT)
        {
            for (uint i = 0; i < taskGroups; i++)
            {
                MeshTaskCommand command;
                command.drawId = primitiveDrawDataIndex;
                command.taskCount = min(mesh.meshletCount - i * NUM_AS_THREADS, NUM_AS_THREADS);
                command.meshletOffset = i * NUM_AS_THREADS;
                
                constants.taskCommands.Store(drawOffset + i, command);
            }
        }
    }
}