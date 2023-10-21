#include "Structures.hlsli"

#ifndef COMMON_BUFFERS_H
#define COMMON_BUFFERS_H

ConstantBuffer<CameraData> u_cameraData : register(b0, space2);
ConstantBuffer<DirectionalLight> u_directionalLight : register(b1, space2);
ConstantBuffer<SamplersData> u_samplers : register(b2, space2);

#endif