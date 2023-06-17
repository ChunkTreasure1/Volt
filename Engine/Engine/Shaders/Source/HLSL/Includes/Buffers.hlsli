#include "Common.hlsli"
#include "Defines.hlsli"

#ifndef BUFFERS_H
#define BUFFERS_H

RWByteAddressBuffer u_indirectCounts : register(BINDING_INDIRECT_COUNT, SPACE_DRAW_BUFFERS);
RWStructuredBuffer<uint> u_drawToObjectId : register(BINDING_DRAW_TO_OBJECT_ID, SPACE_DRAW_BUFFERS);

RWStructuredBuffer<IndirectGPUBatch> u_indirectBatches : register(BINDING_MAIN_INDIRECT_ARGS, SPACE_RENDERER_BUFFERS);
RWStructuredBuffer<int> u_visiblePointLightIndices : register(BINDING_VISIBLE_POINT_LIGHTS, SPACE_RENDERER_BUFFERS);
RWStructuredBuffer<int> u_visibleSpotLightIndices : register(BINDING_VISIBLE_SPOT_LIGHTS, SPACE_RENDERER_BUFFERS);

StructuredBuffer<GPUMesh> u_meshes : register(BINDING_MESHTABLE, SPACE_MAINBUFFERS);

StructuredBuffer<ObjectData> u_objectData : register(BINDING_OBJECT_DATA, SPACE_RENDERER_BUFFERS);
StructuredBuffer<PointLight> u_pointLights : register(BINDING_POINT_LIGHTS, SPACE_RENDERER_BUFFERS);
StructuredBuffer<SpotLight> u_spotLights : register(BINDING_SPOT_LIGHTS, SPACE_RENDERER_BUFFERS);
StructuredBuffer<SphereLight> u_sphereLights : register(BINDING_SPHERE_LIGHTS, SPACE_RENDERER_BUFFERS);
StructuredBuffer<RectangleLight> u_rectangleLights : register(BINDING_RECTANGLE_LIGHTS, SPACE_RENDERER_BUFFERS);
StructuredBuffer<float4x4> u_animationData : register(BINDING_ANIMATION_DATA, SPACE_RENDERER_BUFFERS);
StructuredBuffer<ParticleRenderingInfo> u_particleInfo : register(BINDING_PARTICLE_INFO, SPACE_RENDERER_BUFFERS);
StructuredBuffer<uint> u_paintedVertexColors : register(BINDING_PAINTED_VERTEX_COLORS, SPACE_RENDERER_BUFFERS);

ConstantBuffer<CameraData> u_cameraData : register(BINDING_CAMERABUFFER, SPACE_RENDERER_BUFFERS);
ConstantBuffer<DirectionalLight> u_directionalLight : register(BINDING_DIRECTIONAL_LIGHT, SPACE_RENDERER_BUFFERS);
ConstantBuffer<SceneData> u_sceneData : register(BINDING_SCENE_DATA, SPACE_RENDERER_BUFFERS);
ConstantBuffer<RendererData> u_rendererData : register(BINDING_RENDERER_DATA, SPACE_RENDERER_BUFFERS);

#endif 
