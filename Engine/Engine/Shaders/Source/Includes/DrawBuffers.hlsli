#pragma once

#include "Defines.hlsli"
#include "Structures.hlsli"
#include "Material.hlsli"

///// Draw buffers /////
StructuredBuffer<uint> u_drawToInstanceOffset : register(t0, space0);
StructuredBuffer<IndirectDrawData> u_indirectDrawData : register(t1, space0);
StructuredBuffer<uint> u_instanceOffsetToObjectID : register(t2, space0);
StructuredBuffer<Material> u_materials : register(t3, space0);
////////////////////////

IndirectDrawData GetDrawData(uint objectId)
{
    return u_indirectDrawData[objectId];
}

Material GetMaterial(uint materialId)
{
    return u_materials[materialId];
}