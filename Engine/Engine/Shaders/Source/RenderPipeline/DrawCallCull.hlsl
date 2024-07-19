#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Structures.hlsli"

#include "MeshShaderConfig.hlsli"
#include "Utility.hlsli"

struct Constants
{
    UniformRWTypedBuffer<uint> countBuffer;
    UniformRWTypedBuffer<MeshTaskCommand> taskCommands;

    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;

    uint drawCallCount;
};

[numthreads(64, 1, 1)]
void MainCS(uint dispatchThreadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (dispatchThreadId >= constants.drawCallCount)
    {
        return;
    }

    const ViewData viewData = constants.viewData.Load();

    const ObjectDrawData drawData = constants.gpuScene.objectDrawDataBuffer.Load(dispatchThreadId);
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);

    const float3 center = mul(viewData.view, float4(drawData.transform.GetWorldPosition(mesh.boundingSphere.center), 1.f)).xyz;
    const float radius = mesh.boundingSphere.radius * max(drawData.transform.scale.x, max(drawData.transform.scale.y, drawData.transform.scale.z));

    bool visible = true;

    visible = visible && center.z * viewData.cullingFrustum.y - abs(center.x) * viewData.cullingFrustum.x > -radius;
    visible = visible && center.z * viewData.cullingFrustum.w - abs(center.y) * viewData.cullingFrustum.z > -radius;
    
    visible = visible && center.z + radius > viewData.nearPlane && center.z - radius < viewData.farPlane;

    if (visible)
    {
        uint taskGroups = DivideRoundUp(mesh.meshletCount, NUM_AS_THREADS);
        
        uint drawOffset;
        constants.countBuffer.InterlockedAdd(0, taskGroups, drawOffset);

        // Skip draw calls if AS group count limit is reached. This equals ~4M visible draws or ~32B visible triangles.
        if (drawOffset + taskGroups <= NUM_MAX_AS_GROUP_COUNT)
        {
            for (uint i = 0; i < taskGroups; i++)
            {
                MeshTaskCommand command;
                command.drawId = dispatchThreadId;
                command.taskCount = min(mesh.meshletCount - i * NUM_AS_THREADS, NUM_AS_THREADS);
                command.meshletOffset = i * NUM_AS_THREADS;
                
                constants.taskCommands.Store(drawOffset + i, command);
            }
        }
    }
}