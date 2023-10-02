#include "DecalCommon.hlsli"

DecalOutput main(in DefaultVertexInput input)
{
    const float4 worldPosition = mul(u_materialData.transform, float4(input.position, 1.f));
    
    DecalOutput output = (DecalOutput) 0;
    output.worldPosition = worldPosition.xyz;
    output.position = mul(u_cameraData.projection, mul(u_cameraData.view, worldPosition));
    output.texCoords = input.texCoords;
    
    return output;
}