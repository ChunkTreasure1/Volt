#pragma once

#include "Volt/Rendering/RayTracingCommon.h"
#include "Volt/Asset/Asset.h"

#include <Wire/Wire.h>

#include <set>

namespace Volt
{
	class Scene;
	class Mesh;

	struct RayTracedSceneRendererSpecification
	{
		Weak<Scene> scene;
	};

	struct BottomLevelAccelerationStructures
	{
		std::vector<AccelerationStructure> accelerationStructures;
		std::unordered_map<AssetHandle, size_t> meshToIndexMap;
	};

	struct TopLevelAccelerationStructures
	{
		std::vector<AccelerationStructure> accelerationStructures;
	};

	class RayTracedSceneRenderer
	{
	public:
		RayTracedSceneRenderer(const RayTracedSceneRendererSpecification& specification);
		~RayTracedSceneRenderer();

		void BuildBottomLevelAccelerationStructures();
		void BuildTopLevelAccelerationStructures();

	private:
		AccelerationStructure BuildBottomLevelAccelerationStructure(Ref<Mesh> mesh);
		AccelerationStructure BuildTopLevelAccelerationStructure(Wire::EntityId id);

		AccelerationStructure CreateAccelerationStructure(VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
		void DestroyAccelerationStructure(AccelerationStructure accelerationStruct);

		ScratchBuffer CreateScratchBuffer(VkDeviceSize deviceSize);
		void DestroyScratchBuffer(ScratchBuffer scratchBuffer);

		RayTracedSceneRendererSpecification mySpecification;

		BottomLevelAccelerationStructures myBLAS;
		TopLevelAccelerationStructures myTLAS;
		Weak<Scene> myScene;
	};
}
