#include "Resources.hlsli"
#include "GPUScene.hlsli"

struct Constants
{
    vt::RWTypedBuffer<uint> outputBuffer;
    vt::TypedBuffer<GPUMesh> inputBuffer;
};

[numthreads(1, 1, 1)]
void main(uint threadId : SV_DispatchThreadID)
{ 
    const Constants constants = GetConstants<Constants>();
    
    const GPUMesh mesh = constants.inputBuffer.Load(0);
    const float3 pos = mesh.vertexPositionsBuffer.Load(0);

    constants.outputBuffer.Store(0, (uint)pos.x);
} 