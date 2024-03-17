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

struct IndirectIndexedCommand
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
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

struct ViewData
{
    // Camera
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float4x4 inverseProjection;
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    float4 cameraPosition;
    float2 depthUnpackConsts;
    float nearPlane;
    float farPlane;
	
    // Render Target
    float2 renderSize;
    float2 invRenderSize;
};

struct DirectionalLight
{
    float4 direction;
    float3 color;
    float intensity;
};

#endif