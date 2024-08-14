#include "PushConstant.hlsli"
#include "MeshShaderCommon.hlsli"

groupshared MeshAmplificationPayload m_payload;

[numthreads(NUM_AS_THREADS, 1, 1)]
void MainAS(uint groupThreadId : SV_GroupThreadID, uint2 groupId : SV_GroupID)
{
    const Constants constants = GetConstants<Constants>();
 
    const uint taskIndex = groupId.x * NUM_AS_THREADS + groupId.y;

    const MeshTaskCommand command = constants.taskCommands.Load(taskIndex);
    const PrimitiveDrawData drawData = constants.gpuScene.primitiveDrawDataBuffer.Load(command.drawId);    
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);

    const uint meshletIndex = command.meshletOffset + groupThreadId;

    bool visible = false;

    if (groupThreadId < command.taskCount)
    {
        const Meshlet meshlet = mesh.meshletsBuffer.Load(mesh.meshletStartOffset + meshletIndex);       
        const ViewData viewData = constants.viewData.Load(); 
        
        const float3 center = drawData.transform.GetWorldPosition(meshlet.boundingSphereCenter);
        const float3 viewCenter = mul(viewData.view, float4(center, 1.f)).xyz;
        const float radius = meshlet.boundingSphereRadius * max(drawData.transform.scale.x, max(drawData.transform.scale.y, drawData.transform.scale.z));
        
        const float3 coneAxis = drawData.transform.RotateVector(meshlet.GetConeAxis());
        const float coneCutoff = meshlet.GetConeCutoff();        

        visible = true; //!ConeCull(center, radius, coneAxis, coneCutoff, viewData.cameraPosition.xyz);
        visible = visible && viewCenter.z * viewData.cullingFrustum.y - abs(viewCenter.x) * viewData.cullingFrustum.x > -radius;
        visible = visible && viewCenter.z * viewData.cullingFrustum.w - abs(viewCenter.y) * viewData.cullingFrustum.z > -radius;
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        m_payload.meshletIndices[index] = meshletIndex;
        m_payload.drawId = command.drawId;
    }

    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, m_payload);
}