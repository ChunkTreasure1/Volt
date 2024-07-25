#pragma once

#include "Resources.hlsli"
#include "BoundingVolumes.hlsli"

struct MeshSDF
{
    BoundingBox boundingBox;
    TTexture<float> sdfTexture;
};