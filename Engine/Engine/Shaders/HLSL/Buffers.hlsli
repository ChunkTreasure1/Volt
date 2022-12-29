cbuffer CameraBuffer : register(b0)
{
	CameraData u_cameraData;
}

cbuffer ObjectBuffer : register(b1)
{
	ObjectData u_objectData;
}

cbuffer PassBuffer : register(b3)
{
	PassData u_passData;
}

cbuffer DirectionalLightBuffer : register(b4)
{
	DirectionalLight u_directionalLight;
}

cbuffer AnimationBuffer : register(b6)
{
	AnimationData u_animationData;
}

cbuffer SceneBuffer : register(b7)
{
	SceneData u_sceneData;
}

StructuredBuffer<ObjectData> u_instanceData : register(t100);
StructuredBuffer<float4x4> u_instanceAnimationData : register(t101); // Max 128 bones per mesh
StructuredBuffer<PointLight> u_pointLights : register(t102);