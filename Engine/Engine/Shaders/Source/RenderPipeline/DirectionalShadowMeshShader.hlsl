#include "PushConstant.hlsli"
#include "Resources.hlsli"
#include "GPUScene.hlsli"
#include "Lights.hlsli"
#include "Structures.hlsli"

struct Constants
{
    GPUScene gpuScene;
    UniformBuffer<DirectionalLight> directionalLight;
    UniformTypedBuffer<MeshTaskCommand> taskCommands;

    float4x4 viewMatrix;
    float4 cullingFrustum;
    uint2 renderSize;
};

#define OVERRIDE_DEFAULT_CONSTANTS
#include "MeshShaderCommon.hlsli"

struct PerDrawData
{
    uint viewIndex;
};

PUSH_CONSTANT(PerDrawData, u_perDrawData);

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
        
        const float3 center = drawData.transform.GetWorldPosition(meshlet.boundingSphereCenter);
        const float3 viewCenter = mul(constants.viewMatrix, float4(center, 1.f)).xyz;
        const float radius = meshlet.boundingSphereRadius * max(drawData.transform.scale.x, max(drawData.transform.scale.y, drawData.transform.scale.z));
        
        const float3 coneAxis = drawData.transform.RotateVector(meshlet.GetConeAxis());
        const float coneCutoff = meshlet.GetConeCutoff();        

        visible = true; //!ConeCull(center, radius, coneAxis, coneCutoff, viewData.cameraPosition.xyz);

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
        uint index = WavePrefixCountBits(visible);
        m_payload.meshletIndices[index] = meshletIndex;
        m_payload.drawId = command.drawId;
    }

    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, m_payload);
}

#include "MeshShaderCullCommon.hlsli"

struct VertexOutput
{
    float4 position : SV_Position;
};

struct PrimitiveOutput
{
    uint target : SV_RenderTargetArrayIndex;
    bool cullPrimitive : SV_CullPrimitive;
};

[numthreads(NUM_MS_THREADS, 1, 1)]
[outputtopology("triangle")]
void MainMS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
            in payload MeshAmplificationPayload payload,
            out indices uint3 tris[NUM_MAX_OUT_TRIS],
            out vertices VertexOutput vertices[NUM_MAX_OUT_VERTS],
            out primitives PrimitiveOutput primitives[NUM_MAX_OUT_TRIS])
{
    const Constants constants = GetConstants<Constants>();
    const DirectionalLight dirLight = constants.directionalLight.Load();

    const PrimitiveDrawData drawData = constants.gpuScene.primitiveDrawDataBuffer.Load(payload.drawId);    
    const GPUMesh mesh = constants.gpuScene.meshesBuffer.Load(drawData.meshId);

    uint meshletIndex = payload.meshletIndices[groupId];
    if (meshletIndex >= mesh.meshletCount)
    {
        return;
    }

    const Meshlet meshlet = mesh.meshletsBuffer.Load(mesh.meshletStartOffset + meshletIndex);
    const uint vertexCount = meshlet.GetVertexCount();
    const uint triCount = meshlet.GetTriangleCount();

    SetMeshOutputCounts(vertexCount, triCount);

    if (groupThreadId < vertexCount)
    {
        const uint vertexIndex = mesh.meshletDataBuffer[meshlet.GetVertexOffset() + groupThreadId] + mesh.vertexStartOffset;
        float4x4 skinningMatrix = IDENTITY_MATRIX;
        if (drawData.isAnimated)
        {
            skinningMatrix = GetSkinningMatrix(mesh, vertexIndex, drawData.boneOffset, constants.gpuScene.bonesBuffer);
        }

        const float3 skinnedPosition = mul(skinningMatrix, float4(mesh.vertexPositionsBuffer.Load(vertexIndex), 1.f)).xyz;
        const float4 position = mul(dirLight.viewProjections[u_perDrawData.viewIndex], float4(drawData.transform.GetWorldPosition(skinnedPosition), 1.f));

        SetupCullingPositions(groupThreadId, position, constants.renderSize);

        vertices[groupThreadId].position = TransformClipPosition(position);
    }

    if (groupThreadId < triCount)
    {
        const uint primitive = mesh.meshletDataBuffer.Load(meshlet.GetIndexOffset() + groupThreadId);
        const uint3 indices = UnpackPrimitive(primitive);        

        tris[groupThreadId] = indices;
        primitives[groupThreadId].target = u_perDrawData.viewIndex;
        primitives[groupThreadId].cullPrimitive = IsPrimitiveCulled(indices);
    }
}

struct ColorOutput
{
    [[vt::d32f]];
};

struct VertexInput
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;
};

ColorOutput MainPS(VertexInput input)
{
    ColorOutput output;
    return output;
}