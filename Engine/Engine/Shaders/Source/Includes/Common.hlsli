#ifdef __GLSL__

#define int2 ivec2
#define int3 ivec3
#define int4 ivec4

#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4

#define double2 dvec2
#define double3 dvec3
#define double4 dvec4

#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define float4x4 mat4
#define float3x3 mat3
#define float2x3 mat2

#endif // __GLSL__

#ifndef COMMON_H
#define COMMON_H

///// Constant Buffers /////
struct CameraData
{
	float4x4 view;
	float4x4 projection;
    float4x4 viewProjection;
	
	float4x4 inverseView;
	float4x4 inverseProjection;
    
    float4x4 nonReversedProj;
    float4x4 inverseNonReverseViewProj;
    
	float4 position;
	
	float nearPlane;
	float farPlane;
    float2 depthUnpackConsts;
};

struct SceneData
{
    float timeSinceStart;
    float deltaTime;
    uint pointLightCount;
    uint spotLightCount;
    
    uint sphereLightCount;
    uint rectangleLightCount;
    float ambianceIntensity;
    uint padding;
};

struct RendererData
{
    uint screenTilesCountX;
    uint enableAO;
    uint2 padding;
};

////// Data Structures /////
struct ObjectData
{
    float4x4 transform;
	
    uint id;
    uint isAnimated;
    float timeSinceCreation;
    float boundingSphereRadius;
	
    float3 boundingSphereCenter;
    uint materialIndex;
    
    float randomValue;
    uint boneOffset;
    int colorOffset;
    uint padding;
};

struct IndirectGPUBatch
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	int vertexOffset;
	uint firstInstance;

    uint objectId;
    uint batchId;
    uint padding;
};

struct ParticleRenderingInfo
{
    float3 position;
    float randomValue;
    
    float4 color;
    
    float2 scale;
    uint albedoIndex;
    uint normalIndex;
    
    uint materialIndex;
    float timeSinceSpawn;
    float2 padding2;
};

///// Lights //////
struct PointLight
{
    float4 position;
    float4 color;

    float intensity;
    float radius;
    float falloff;
    int shadowMapIndex;
	
    float3 padding2;
    uint castShadows;

    float4x4 viewProjectionMatrices[6];
};

struct SpotLight
{
    float3 position;
    float intensity;
    
    float3 color;
    float angleAttenuation;
    
    float3 direction;
    float range;
    
    float angle;
    float falloff;
    uint castShadows;
    int shadowMapIndex;
    
    float4x4 viewProjection;
};

struct SphereLight
{
    float3 position;
    float intensity;
    
    float3 color;
    float radius;
};

struct RectangleLight
{
    float3 direction;
    float intensity;
    
    float3 left;
    float width;
    
    float3 up;
    float height;
    
    float3 position;
    float padding;
    
    float3 color;
    float padding2;
};

struct DirectionalLight
{
    float4 direction;
    float4 colorIntensity;

    float4x4 viewProjections[5];
    float4 cascadeDistances[5];
    
    uint castShadows;
    uint softShadows;
    float lightSize;
    uint padding;
};

#endif