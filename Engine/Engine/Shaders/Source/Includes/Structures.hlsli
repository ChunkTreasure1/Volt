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
    uint meshletId;
    uint padding;
};

struct IndirectDrawData
{
    float4x4 transform;
    uint meshId;
    uint vertexStartOffset;
    uint materialId;
    
    uint padding;
};
    
///// Rendering Structures /////
struct CommonResourcesData
{
};

struct SamplersData
{
    uint linearSampler;
    uint linearPointSampler;
    
    uint pointSampler;
    uint pointLinearSampler;
    
    uint linearClampSampler;
    uint linearPointClampSampler;
    
    uint pointClampSampler;
    uint pointLinearClampSampler;
    
    uint anisotropicSampler;
    
    uint3 padding;
};

struct CameraData
{
    float4x4 view;
    float4x4 projection;

    float4x4 inverseView;
    float4x4 inverseProjection;

    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    
    float4 position;
    
    float2 depthUnpackConsts;
    float nearPlane;
    float farPlane;
};

struct DirectionalLight
{
    float4 direction;
    
    float3 color;
    float intensity;
};

#endif