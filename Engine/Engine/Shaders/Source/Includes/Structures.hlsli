#ifndef STRUCTURES_H
#define STRUCTURES_H

struct IndirectGPUCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
    
    uint objectId;
    uint meshId;
    uint2 padding;
};

struct IndirectDrawData
{
    float4x4 transform;
    uint meshId;
    uint vertexStartOffset;
    
    uint2 padding;
};

///// Rendering Structures /////
struct CameraData
{
    float4x4 view;
    float4x4 projection;

    float4x4 inverseView;
    float4x4 inverseProjection;

    float4 position;
};

struct DirectionalLight
{
    float4 direction;
    
    float3 color;
    float intensity;
};

#endif