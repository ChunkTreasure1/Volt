#define __GLSL__

#include "Common.hlsli" //! #include "../../Includes/Common.hlsli"
#include "Defines.glsl"

#ifndef BUFFERS_H
#define BUFFERS_H

layout(std140, set = SPACE_RENDERER_BUFFERS, binding = BINDING_CAMERABUFFER) uniform CameraBuffer
{
	CameraData u_cameraData;
};

layout(std140, set = SPACE_RENDERER_BUFFERS, binding = BINDING_DIRECTIONAL_LIGHT) uniform DirectionalLightBuffer
{
	DirectionalLight u_directionalLight;
};

layout(std140, set = SPACE_RENDERER_BUFFERS, binding = BINDING_SCENE_DATA) uniform SceneDataBuffer
{
	SceneData u_sceneData;
};

layout(std140, set = SPACE_RENDERER_BUFFERS, binding = BINDING_RENDERER_DATA) uniform RendererDataBuffer
{
	RendererData u_rendererData;
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_INDIRECT_ARGS) buffer IndirectBatchesBuffer
{
	IndirectGPUBatch u_indirectBatches[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_INDIRECT_COUNT) buffer IndirectCountsBuffer
{
	uint u_indirectCounts[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_DRAW_TO_OBJECT_ID) buffer DrawToObjectIDBuffer
{
	uint u_drawToObjectId[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_VISIBLE_POINT_LIGHTS) buffer VisiblePointLightsIndicesBuffer
{
	int u_visiblePointLightIndices[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_VISIBLE_SPOT_LIGHTS) buffer VisibleSpotLightsIndicesBuffer
{
	int u_visibleSpotLightIndices[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_OBJECT_DATA) buffer ObjectDataBuffer
{
	ObjectData u_objectData[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_POINT_LIGHTS) buffer PointLightsBuffer
{
	PointLight u_pointLights[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_SPOT_LIGHTS) buffer SpotLightsBuffer
{
	SpotLight u_spotLights[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_SPHERE_LIGHTS) buffer SphereLightsBuffer
{
	SphereLight u_sphereLights[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_RECTANGLE_LIGHTS) buffer RectangleLightsBuffer
{
	RectangleLight u_rectangleLights[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_ANIMATION_DATA) buffer AnimationDataBuffer
{
	float4x4 u_animationData[];
};

layout(std430, set = SPACE_RENDERER_BUFFERS, binding = BINDING_PARTICLE_INFO) buffer ParticleInfoBuffer
{
	ParticleRenderingInfo u_particleInfo[];
};

#endif