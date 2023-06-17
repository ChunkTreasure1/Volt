#pragma once

#include "Volt/Asset/Mesh/SubMesh.h"

#include <gem/gem.h>
#include <vulkan/vulkan.h>

namespace Volt
{
	class SubMaterial;
	class Mesh;
	class Material;

	///// Structures /////
	struct PointLight
	{
		gem::vec4 position;
		gem::vec4 color;

		float intensity;
		float radius;
		float falloff;
		int32_t shadowMapIndex = -1;

		gem::vec3 padding2;
		uint32_t castShadows = 0;
		gem::mat4 viewProjectionMatrices[6];
	};

	struct SpotLight
	{
		gem::vec3 position;
		float intensity;

		gem::vec3 color;
		float angleAttenuation;

		gem::vec3 direction;
		float range;

		float angle;
		float falloff;
		uint32_t castShadows = 0;
		int32_t shadowMapIndex = -1;

		gem::mat4 viewProjection = { 1.f };
		gem::mat4 projection = { 1.f };
	};

	struct SphereLight
	{
		gem::vec3 position;
		float intensity;

		gem::vec3 color;
		float radius;
	};

	struct RectangleLight
	{
		gem::vec3 direction;
		float intensity;

		gem::vec3 left;
		float width;

		gem::vec3 up;
		float height;

		gem::vec3 position;
		float padding;

		gem::vec3 color;
		float padding2;
	};

	struct DirectionalLight
	{
		gem::vec4 direction = { 0.f, 0.f, 0.f, 0.f };
		gem::vec4 colorIntensity = { 0.f, 0.f, 0.f, 0.f };

		gem::mat4 viewProjections[4];
		gem::vec4 cascadeDistances[4];

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

		gem::vec4 color = 1.f;
		gem::vec3 emissiveColor = 1.f;
		float emissiveStrength = 1.f;

		float roughness = 0.5f;
		float metalness = 0.f;
		float normalStrength = 0.f;
		float padding1 = 0.f;
	};

	struct GPUMeshLOD
	{
		uint32_t indexCount = 0;
		uint32_t indexOffset = 0;
	};

	struct GPUMesh
	{
		inline static constexpr uint32_t MAX_LOD_COUNT = 8;

		uint32_t lodCount = 0;
		gem::vec3i padding = 0;

		GPUMeshLOD lods[MAX_LOD_COUNT];
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
		gem::vec3 position;
		float randomValue;

		gem::vec4 color;

		gem::vec2 scale;
		uint32_t albedoIndex = 0;
		uint32_t normalIndex = 0;

		uint32_t materialIndex = 0;
		float timeSinceSpawn = 0.f;
		gem::vec2 padding2;
	};

	struct ParticleBatch
	{
		std::vector<ParticleRenderingInfo> particles;
		Ref<Material> material;
		uint32_t startOffset = 0;

		gem::vec3 emitterPosition = 0.f;
	};

	///// Buffers /////
	struct CameraData
	{
		gem::mat4 view;
		gem::mat4 proj;
		gem::mat4 viewProjection;

		gem::mat4 inverseView;
		gem::mat4 inverseProj;

		gem::mat4 nonReversedProj;
		gem::mat4 inverseNonReverseViewProj;

		gem::vec4 position;

		float nearPlane;
		float farPlane;
		gem::vec2 depthUnpackConsts;

		gem::vec2 NDCToViewMul;
		gem::vec2 NDCToViewAdd;
	};

	struct RendererGPUData
	{
		uint32_t screenTilesCountX;
		uint32_t enableAO;
		gem::vec2ui padding;
	};

	struct ObjectData
	{
		gem::mat4 transform;

		uint32_t id;
		uint32_t isAnimated;
		float timeSinceCreation = 0.f;
		float boundingSphereRadius = 0.f;

		gem::vec3 boundingSphereCenter = 0.f;
		uint32_t materialIndex = 0;

		float randomValue = 0.f;
		uint32_t boneOffset = 0;		
		int32_t colorOffset = -1;
		uint32_t meshIndex = 0;
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
		gem::vec3 globalColor = { 1.f };
	};

	struct GTAOSettings
	{
		float radius = 50.f;
		float radiusMultiplier = 1.457f;
		float falloffRange = 0.615f;
		float finalValuePower = 2.2f;
	};
}
