#include "PushConstant.hlsli"
#include "MeshShaderCommon.hlsli"

struct PerDrawData
{
    uint drawIndex;
};

PUSH_CONSTANT(PerDrawData, u_perDrawData);

groupshared MeshAmplificationPayload m_payload;

[numthreads(NUM_AS_THREADS, 1, 1)]
void MainAS(uint groupThreadId : SV_GroupThreadID, uint dispatchThreadId : SV_DispatchThreadID, uint groupId : SV_GroupID)
{
    const Constants constants = GetConstants<Constants>();
    const ObjectDrawData drawData = constants.gpuScene.objectDrawDataBuffer.Load(u_perDrawData.drawIndex);    
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);

    bool visible = false;

    if (dispatchThreadId < mesh.meshletCount)
    {
        const Meshlet meshlet = mesh.meshletsBuffer.Load(mesh.meshletStartOffset + dispatchThreadId);       
        const ViewData viewData = constants.viewData.Load(); 
        
        const float3 center = drawData.transform.GetWorldPosition(meshlet.boundingSphereCenter);
        const float3 viewCenter = mul(viewData.view, float4(center, 1.f)).xyz;
        const float radius = meshlet.boundingSphereRadius * max(drawData.transform.scale.x, max(drawData.transform.scale.y, drawData.transform.scale.z));
        
        const float3 coneAxis = drawData.transform.RotateVector(meshlet.GetConeAxis());
        const float coneCutoff = meshlet.GetConeCutoff();        

        visible = !ConeCull(center, radius, coneAxis, coneCutoff, viewData.cameraPosition.xyz);
        visible = visible && viewCenter.z * viewData.cullingFrustum.y - abs(viewCenter.x) * viewData.cullingFrustum.x > -radius;
        visible = visible && viewCenter.z * viewData.cullingFrustum.w - abs(viewCenter.y) * viewData.cullingFrustum.z > -radius;
        visible = true;    
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        m_payload.meshletIndices[index] = dispatchThreadId;
        m_payload.drawId = u_perDrawData.drawIndex;
    }

    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, m_payload);
}