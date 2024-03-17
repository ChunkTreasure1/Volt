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
    
    // Temp lights
    uint pointLightCount;
    uint spotLightCount;
};

#define DIRECTIONAL_SHADOW_CASCADE_COUNT 5

struct DirectionalLight
{
    float4 direction;
    float3 color;
    float intensity;

    uint enableShadows;

    float cascadeDistances[DIRECTIONAL_SHADOW_CASCADE_COUNT];
    float4x4 viewProjections[DIRECTIONAL_SHADOW_CASCADE_COUNT];
};

struct PointLight
{
    float3 position;
    float radius;
    
    float3 color;
    float intensity;
    
    float falloff;
    float3 padding;
};

struct SpotLight
{
    float3 position;
    float angleAttenuation;
    
    float3 color;
    float intensity;
    
    float3 direction;
    float range;
    
    float angle;
    float falloff;
    float2 padding;
};

#endif