#include "Vertex.hlsli"

struct Input
{
    uint instanceId : SV_InstanceID;
};

DefaultParticleVertex main(Input input)
{
    DefaultParticleVertex output;
    output.instanceId = input.instanceId;
    
    return output;
}