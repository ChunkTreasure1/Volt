#pragma once

#include "Volt/Asset/Mesh/SubMesh.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Volt
{
	class SubMaterial;
	class Mesh;
	class Material;

	///// Structures /////
	struct PointLight
	{
		glm::vec4 position;
		glm::vec4 color;

		float intensity;
		float radius;
		float falloff;
		int32_t shadowMapIndex = -1;

		glm::vec3 padding2;
		uint32_t castShadows = 0;
		glm::mat4 viewProjectionMatrices[6];
	};

	struct SpotLight
	{
		glm::vec3 position;
		float intensity;

		glm::vec3 color;
		float angleAttenuation;

		glm::vec3 direction;
		float range;

		float angle;
		float falloff;
		uint32_t castShadows = 0;
		int32_t shadowMapIndex = -1;

		glm::mat4 viewProjection = { 1.f };
	};

	struct SphereLight
	{
		glm::vec3 position;
		float intensity;

		glm::vec3 color;
		float radius;
	};

	struct RectangleLight
	{
		glm::vec3 direction;
		float intensity;

		glm::vec3 left;
		float width;

		glm::vec3 up;
		float height;

		glm::vec3 position;
		float padding;

		glm::vec3 color;
		float padding2;
	};

	struct DirectionalLight
	{
		glm::vec4 direction = { 0.f, 0.f, 0.f, 0.f };
		glm::vec4 colorIntensity = { 0.f, 0.f, 0.f, 0.f };

		glm::mat4 viewProjections[5];

		glm::vec4 cascadeDistances[5];

		uint32_t castShadows = 1;
		uint32_t softShadows = 1;
		float lightSize = 1.f;
		uint32_t padding;
	};

	struct GPUMaterial
	{
		uint32_t albedoTexture = 0;
		uint32_t normalTexture = 0;
		uint32_t materialTexture = 0;
		uint32_t materialFlags = 0;

		glm::vec4 color = 1.f;
		glm::vec3 emissiveColor = 1.f;
		float emissiveStrength = 1.f;

		float roughness = 0.5f;
		float metalness = 0.f;
		float normalStrength = 0.f;
		float padding1 = 0.f;
	};

	struct EnvironmentProbeInfo
	{
		uint32_t priority;
		glm::vec3 center;

		glm::vec4 size;
		glm::mat4 transform;

		float diffuseMultiplier = 1.f;
		float specularMultiplier = 1.f;
		float falloff = 1.f;
		float blendRadius;
	};

	struct FogVolume
	{
		glm::vec3 center;
		float density;

		glm::vec4 size;
		glm::mat4 transform;

		glm::vec3 color;
		float padding;
	};

	///// Indirect /////
	struct IndirectBatch
	{
		Ref<SubMaterial> material = nullptr;
		Ref<Mesh> mesh = nullptr;

		SubMesh subMesh{};

		uint32_t first{};
		uint32_t count{};
		uint32_t id{};
	};

	struct IndirectDrawCall
	{
		uint32_t objectId = 0;
		uint32_t batchId = 0;
		uint32_t materialFlags = 0;

		uint32_t padding = { 0 };
	};

	struct IndirectGPUCommand
	{
		VkDrawIndexedIndirectCommand command{};
		uint32_t objectId{};
		uint32_t batchId{};
		uint32_t padding{};
	};

	struct ParticleRenderingInfo
	{
		glm::vec3 position;
		float randomValue;

		glm::vec4 color;

		glm::vec2 scale;
		uint32_t albedoIndex = 0;
		uint32_t normalIndex = 0;

		uint32_t materialIndex = 0;
		float timeSinceSpawn = 0.f;
		glm::vec2 padding2;
	};

	struct ParticleBatch
	{
		std::vector<ParticleRenderingInfo> particles;
		Ref<Material> material;
		uint32_t startOffset = 0;

		glm::vec3 emitterPosition = 0.f;
	};

	///// Buffers /////
	struct CameraData
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProjection;

		glm::mat4 inverseView;
		glm::mat4 inverseProj;

		glm::mat4 nonReversedProj;
		glm::mat4 inverseNonReverseViewProj;

		glm::vec4 position;

		float nearPlane;
		float farPlane;
		glm::vec2 depthUnpackConsts;
	};

	struct RendererGPUData
	{
		uint32_t screenTilesCountX;
		uint32_t enableAO;
		glm::uvec2 padding;
	};

	struct ObjectData
	{
		glm::mat4 transform;

		uint32_t id;
		uint32_t isAnimated;
		float timeSinceCreation = 0.f;
		float boundingSphereRadius = 0.f;

		glm::vec3 boundingSphereCenter = 0.f;
		uint32_t materialIndex = 0;

		float randomValue = 0.f;
		uint32_t boneOffset = 0;		
		int32_t colorOffset = -1;
		uint32_t padding = 0;
	};

	struct SceneData
	{
		float timeSinceStart = 0.f;
		float deltaTime = 0.f;
		uint32_t pointLightCount = 0;
		uint32_t spotLightCount = 0;

		uint32_t sphereLightCount = 0;
		uint32_t rectangleLightCount = 0;
		float ambianceMultiplier = 1.f;
		uint32_t padding;
	};

	///// Settings /////
	struct VolumetricFogSettings
	{
		float anisotropy = 0.f;
		float density = 0.04f;
		float globalDensity = 0.f;
		glm::vec3 globalColor = { 1.f };
	};

	struct GTAOSettings
	{
		float radius = 50.f;
		float radiusMultiplier = 1.457f;
		float falloffRange = 0.615f;
		float finalValuePower = 2.2f;
	};
}
