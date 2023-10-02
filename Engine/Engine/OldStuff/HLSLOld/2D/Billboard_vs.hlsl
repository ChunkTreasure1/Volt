#include "Vertex.hlsli"

DefaultBillboardVertex main(DefaultBillboardVertex input)
{
    DefaultBillboardVertex output;
    output.position = input.position;
    output.color = input.color;
    output.scale = input.scale;
    output.textureIndex = input.textureIndex;
    output.id = input.id;
    
    return output;
}