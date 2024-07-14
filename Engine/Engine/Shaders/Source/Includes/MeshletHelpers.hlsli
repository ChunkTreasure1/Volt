#pragma once

#define MESHLET_ID_BITS 26u
#define MESHLET_PRIMITIVE_BITS 6u
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

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}