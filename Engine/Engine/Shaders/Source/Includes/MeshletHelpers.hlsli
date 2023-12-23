#pragma once

#define MESHLET_ID_BITS 24u
#define MESHLET_PRIMITIVE_BITS 8u
#define MESHLET_ID_MASK ((1u << MESHLET_ID_BITS) - 1u)
#define MESHLET_PRIMITIVE_MASK ((1u << MESHLET_PRIMITIVE_BITS) - 1u)

uint UnpackTriangleID(uint packedValue)
{
    return packedValue & MESHLET_PRIMITIVE_MASK;
}

uint UnpackMeshletID(uint packedValue)
{
    return (packedValue >> MESHLET_PRIMITIVE_BITS) & MESHLET_ID_MASK;
}

uint PackMeshletAndIndex(uint meshletId, uint index)
{
    return (meshletId << MESHLET_PRIMITIVE_BITS) | ((index) & MESHLET_PRIMITIVE_MASK);
}