#include "Vertex.hlsli"
#include "Buffers.hlsli"
#include "Particle.hlsli"

struct PushConstants
{
    uint particleOffset;
};
[[vk::push_constant]] PushConstants u_pushConstants;

[maxvertexcount(4)]
void main(point DefaultParticleVertex input[1], inout TriangleStream<ParticleData> output)
{
    
    const float2 offsets[4] =
    {
        { -100.f, 100.f },
        { 100.f, 100.f },
        { -100.f, -100.f },
        { 100.f, -100.f }
    };

    const float2 uvs[4] =
    {
        { 0.f, 0.f },
        { 1.f, 0.f },
        { 0.f, 1.f },
        { 1.f, 1.f }
    };
    
    const ParticleRenderingInfo particleInfo = u_particleInfo[u_pushConstants.particleOffset + input[0].instanceId];
    float rand = particleInfo.randomValue;
    float scaleRand = float(1.6) - float(rand);

    float2 newScale = particleInfo.scale.xy*scaleRand;
    
    for (uint i = 0; i < 4; i++)
    {
        ParticleData result;
        result.position = mul(u_cameraData.view, float4(particleInfo.position, 1.f));
        result.position.xy += offsets[i] * newScale;
        result.position = mul(u_cameraData.projection, result.position);
        
        result.worldPosition = particleInfo.position;
        result.color = particleInfo.color;
        result.texCoords = uvs[i];
        
        result.albedoIndex = particleInfo.albedoIndex;
        result.materialIndex = particleInfo.materialIndex;
        result.normalIndex = particleInfo.normalIndex;
        
        result.randomValue = particleInfo.randomValue;
        result.timeSinceSpawn = particleInfo.timeSinceSpawn;
        
        output.Append(result);
    }
}