#pragma once

#include "Structures.hlsli"

///// Vertex Buffers /////
ByteAddressBuffer u_vertexPositionsBuffers[] : register(t0, space1);
ByteAddressBuffer u_vertexMaterialDataBuffers[] : register(t1, space1);
ByteAddressBuffer u_vertexAnimationDataBuffers[] : register(t2, space1);
ByteAddressBuffer u_indexBuffers[] : register(t3, space1);
//////////////////////////
