#include "Structures.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

#define ITERATIONS_PER_GROUP 2
#define THREAD_GROUP_SIZE 32

#define MESHLET_ID_BITS 24u
#define MESHLET_PRIMITIVE_ID_BITS 8u
#define MESHLET_ID_MASK ((1u << MESHLET_ID_BITS) - 1u)
#define MESHLET_PRIMITIVE_ID_MASK ((1u << MESHLET_PRIMITIVE_ID_BITS) - 1u)

struct Constants
{
    RWTypedBuffer<uint> compactedIndexBuffer;
    RWTypedBuffer<IndirectIndexedCommand> drawCommands;
    
    TypedBuffer<Meshlet> gpuMeshlets;
    TypedBuffer<ObjectDrawData> objectDrawDataBuffer;
    TypedBuffer<CameraData> cameraData;
    
    uint meshletCount;
    
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

groupshared bool m_visible = false;

[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint threadId : SV_DispatchThreadID, uint groupId : SV_GroupID, uint groupThreadId : SV_GroupThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (threadId >= constants.meshletCount)
    {
        return;
    }
 
    const uint localTriangleIndex = groupThreadId;
    const uint meshletIndex = groupId;
    
    const Meshlet meshlet = constants.gpuMeshlets.Load(meshletIndex);
    const ObjectDrawData objectData = constants.objectDrawDataBuffer.Load(meshlet.objectId);
    
    // We first start by checking if object is visible
    bool visible;
    if (WaveIsFirstLane())
    {
        visible = IsInFrustum(constants, objectData.boundingSphereCenter, objectData.boundingSphereRadius);
    }
    
    visible = WaveReadLaneFirst(visible);
    
    if (!visible)
    {
        return;
    }
    
    const uint laneIndex = WaveGetLaneIndex();
    
    // Do meshlet culling
    
    // Do triangle culling and compaction
    // Each iteration takes care of half a meshlet
    [unroll]
    for (uint i = 0; i < ITERATIONS_PER_GROUP; ++i)
    {
        const uint primitiveIndex = laneIndex + THREAD_GROUP_SIZE * i;
        if (primitiveIndex < meshlet.triangleCount)
        {
            bool shouldWritePrimitive = true;
        
            uint laneAppendOffset = WavePrefixCountBits(shouldWritePrimitive) * 3;
            uint appendCount = WaveActiveCountBits(shouldWritePrimitive) * 3;
        
            uint appendOffset;
            if (WaveIsFirstLane())
            {
            // Will add to index count parameter
                constants.drawCommands.InterlockedAdd(0, appendCount, appendOffset);
            }
        
            appendOffset = WaveReadLaneFirst(appendOffset);
            appendOffset += laneAppendOffset;
        
            constants.compactedIndexBuffer.Store(appendOffset + 0, (meshletIndex << MESHLET_PRIMITIVE_ID_BITS) | ((primitiveIndex * 3 + 0) & MESHLET_PRIMITIVE_ID_MASK));
            constants.compactedIndexBuffer.Store(appendOffset + 1, (meshletIndex << MESHLET_PRIMITIVE_ID_BITS) | ((primitiveIndex * 3 + 1) & MESHLET_PRIMITIVE_ID_MASK));
            constants.compactedIndexBuffer.Store(appendOffset + 2, (meshletIndex << MESHLET_PRIMITIVE_ID_BITS) | ((primitiveIndex * 3 + 2) & MESHLET_PRIMITIVE_ID_MASK));

        }
    }
}