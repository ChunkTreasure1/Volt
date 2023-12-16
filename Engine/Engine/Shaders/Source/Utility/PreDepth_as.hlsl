#include "Defines.hlsli"
#include "GPUScene.hlsli"
#include "DrawContext.hlsli"

#include "Structures.hlsli"
#include "Utility.hlsli"

#define THREAD_GROUP_SIZE 32

struct Constants
{
    TypedBuffer<GPUScene> gpuScene;
    TypedBuffer<DrawContext> drawContext;
    
    TypedBuffer<CameraData> cameraData; // #TODO_Ivar: Should be uniform buffer
    
    uint meshletCount;
};

struct Payload
{
    uint baseId;
    uint subIds[THREAD_GROUP_SIZE];
};

groupshared Payload m_payload;

[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint dispatchThreadID : SV_DispatchThreadID, uint groupThreadID : SV_GroupThreadID, uint groupId : SV_GroupID)
{
    const Constants constants = GetConstants<Constants>();
    
    const bool render = dispatchThreadID < constants.meshletCount;
    const uint taskCount = WaveActiveCountBits(render);
    
    if (groupThreadID.x == 0)
    {
        m_payload.baseId = groupId * THREAD_GROUP_SIZE;
    }
    
    const uint waveOffset = WavePrefixCountBits(render);
    
    if (render)
    {
        m_payload.subIds[waveOffset] = groupThreadID;
    }
    
    if (groupThreadID.x == 0)
    {
       DispatchMesh(taskCount, 1, 1, m_payload);
    }
}