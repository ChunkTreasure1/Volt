#include "Structures.hlsli"

#ifndef COMMON_BUFFERS_H
#define COMMON_BUFFERS_H

ConstantBuffer<CameraData> u_cameraData : register(b0, space5);
ConstantBuffer<DirectionalLight> u_directionalLight : register(b1, space5);

#endif