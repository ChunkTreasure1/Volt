#include "vtpch.h"
#include "RayTracedSceneRenderer.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"

#include "Volt/Rendering/Buffer/VertexBuffer.h"
#include "Volt/Rendering/Buffer/IndexBuffer.h"
#include "Volt/Rendering/Buffer/GenericBuffer.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Scene/Scene.h"

#include "Volt/Components/Components.h"


namespace Volt
{
	RayTracedSceneRenderer::RayTracedSceneRenderer(const RayTracedSceneRendererSpecification& specification)
	{
		myScene = specification.scene;
	}

	RayTracedSceneRenderer::~RayTracedSceneRenderer()
	{
		for (const auto& blas : myBLAS.accelerationStructures)
		{
			DestroyAccelerationStructure(blas);
		}

		myBLAS.meshToIndexMap.clear();
		myBLAS.accelerationStructures.clear();

		for (const auto& tlas : myTLAS.accelerationStructures)
		{
			DestroyAccelerationStructure(tlas);
		}

		myTLAS.accelerationStructures.clear();
	}

	void RayTracedSceneRenderer::BuildBottomLevelAccelerationStructures()
	{
		for (const auto& blas : myBLAS.accelerationStructures)
		{
			DestroyAccelerationStructure(blas);
		}

		myBLAS.meshToIndexMap.clear();
		myBLAS.accelerationStructures.clear();

		auto scenePtr = myScene.lock();

		scenePtr->GetRegistry().ForEach<MeshComponent>([&](Wire::EntityId, const MeshComponent& comp)
		{
			if (comp.handle == Asset::Null())
			{
				return;
			}

			if (myBLAS.meshToIndexMap.contains(comp.handle))
			{
				return;
			}

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(comp.handle);
			if (!mesh || !mesh->IsValid())
			{
				return;
			}

			myBLAS.accelerationStructures.emplace_back(BuildBottomLevelAccelerationStructure(mesh));
			myBLAS.meshToIndexMap.emplace(comp.handle, myBLAS.accelerationStructures.size() - 1);
		});
	}

	void RayTracedSceneRenderer::BuildTopLevelAccelerationStructures()
	{
		for (const auto& tlas : myTLAS.accelerationStructures)
		{
			DestroyAccelerationStructure(tlas);
		}

		myTLAS.accelerationStructures.clear();

		auto scenePtr = myScene.lock();
		scenePtr->GetRegistry().ForEach<MeshComponent, TransformComponent>([&](Wire::EntityId id, const MeshComponent& meshComp, const TransformComponent& transComp)
		{
			if (meshComp.handle == Asset::Null() || !transComp.visible)
			{
				return;
			}

			BuildTopLevelAccelerationStructure(id);
		});
	}

	AccelerationStructure RayTracedSceneRenderer::BuildBottomLevelAccelerationStructure(Ref<Mesh> mesh)
	{
		//auto device = GraphicsContextVolt::GetDevice();
		//const auto& vulkanFunctions = Renderer::GetVulkanFunctions();

		VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

		vertexBufferDeviceAddress.deviceAddress = mesh->GetVertexBuffer()->GetDeviceAddress();
		indexBufferDeviceAddress.deviceAddress = mesh->GetIndexBuffer()->GetDeviceAddress();

		//const uint32_t triangleCount = static_cast<uint32_t>(mesh->GetIndices().size()) / 3;
		const uint32_t vertexCount = static_cast<uint32_t>(mesh->GetVertices().size());

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.maxVertex = vertexCount;
		accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(EncodedVertex);
		accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
		accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

		// Get size info
		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
		buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		{
			VkAccelerationStructureBuildGeometryInfoKHR accBuildGeomInfo{};
			accBuildGeomInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			accBuildGeomInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			accBuildGeomInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			accBuildGeomInfo.geometryCount = (uint32_t)mesh->GetSubMeshes().size();
			accBuildGeomInfo.pGeometries = &accelerationStructureGeometry;

			//vulkanFunctions.vkGetAccelerationStructureBuildSizesKHR(device->GetHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accBuildGeomInfo, &triangleCount, &buildSizesInfo);
		}

		AccelerationStructure accelerationStructure = CreateAccelerationStructure(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, buildSizesInfo);
		ScratchBuffer scratchBuffer = CreateScratchBuffer(buildSizesInfo.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR geomBuildInfo{};
		geomBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		geomBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		geomBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		geomBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		geomBuildInfo.dstAccelerationStructure = accelerationStructure.handle;
		geomBuildInfo.geometryCount = (uint32_t)mesh->GetSubMeshes().size();
		geomBuildInfo.pGeometries = &accelerationStructureGeometry;
		geomBuildInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		std::vector<VkAccelerationStructureBuildRangeInfoKHR> accelerationStructureBuildRangeInfos{};
		for (const auto& subMesh : mesh->GetSubMeshes())
		{
			auto& rangeInfo = accelerationStructureBuildRangeInfos.emplace_back();
			rangeInfo.primitiveCount = subMesh.indexCount / 3;
			rangeInfo.primitiveOffset = subMesh.indexStartOffset / 3;
			rangeInfo.firstVertex = subMesh.vertexStartOffset;
			rangeInfo.transformOffset = 0;
		}

		//const VkAccelerationStructureBuildRangeInfoKHR* accInfos = accelerationStructureBuildRangeInfos.data();

		//VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);
		//vulkanFunctions.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &geomBuildInfo, &accInfos);
		//device->FlushCommandBuffer(commandBuffer);

		DestroyScratchBuffer(scratchBuffer);

		return accelerationStructure;
	}

	AccelerationStructure RayTracedSceneRenderer::BuildTopLevelAccelerationStructure(Wire::EntityId id)
	{
		//auto device = GraphicsContextVolt::GetDevice();
		//const auto& vulkanFunctions = Renderer::GetVulkanFunctions();

		auto scenePtr = myScene.lock();

		Entity entity{ id, scenePtr.get() };
		const auto& meshComp = entity.GetComponent<MeshComponent>();

		const auto transMatrix = entity.GetTransform();

		VkTransformMatrixKHR transformMatrix =
		{
			transMatrix[0][0], transMatrix[0][1], transMatrix[0][2], transMatrix[0][3],
			transMatrix[1][0], transMatrix[1][1], transMatrix[1][2], transMatrix[1][3],
			transMatrix[2][0], transMatrix[2][1], transMatrix[2][2], transMatrix[2][3]
		};

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transformMatrix;
		instance.instanceCustomIndex = 0;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = myBLAS.accelerationStructures.at(myBLAS.meshToIndexMap.at(meshComp.handle)).deviceAddress;

		Ref<GenericBuffer> instanceBuffer = GenericBuffer::Create(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, sizeof(VkAccelerationStructureInstanceKHR), &instance);

		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		instanceDataDeviceAddress.deviceAddress = instanceBuffer->GetDeviceAddress();

		VkAccelerationStructureGeometryKHR accStructureGeom{};
		accStructureGeom.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accStructureGeom.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accStructureGeom.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accStructureGeom.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accStructureGeom.geometry.instances.arrayOfPointers = VK_FALSE;
		accStructureGeom.geometry.instances.data = instanceDataDeviceAddress;

		// Get size info
		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
		buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		{
			//uint32_t primitiveCount = 1;

			VkAccelerationStructureBuildGeometryInfoKHR accBuildGeomInfo{};
			accBuildGeomInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			accBuildGeomInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			accBuildGeomInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			accBuildGeomInfo.geometryCount = 1;
			accBuildGeomInfo.pGeometries = &accStructureGeom;

			//vulkanFunctions.vkGetAccelerationStructureBuildSizesKHR(device->GetHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accBuildGeomInfo, &primitiveCount, &buildSizesInfo);
		}

		AccelerationStructure accelerationStructure = CreateAccelerationStructure(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, buildSizesInfo);
		ScratchBuffer scratchBuffer = CreateScratchBuffer(buildSizesInfo.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR geomBuildInfo{};
		geomBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		geomBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		geomBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		geomBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		geomBuildInfo.dstAccelerationStructure = accelerationStructure.handle;
		geomBuildInfo.geometryCount = 1;
		geomBuildInfo.pGeometries = &accStructureGeom;
		geomBuildInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR buildRange{};
		buildRange.primitiveCount = 1;
		buildRange.primitiveOffset = 0;
		buildRange.firstVertex = 0;
		buildRange.transformOffset = 0;

		std::array<VkAccelerationStructureBuildRangeInfoKHR*, 1> ranges = { &buildRange };

		//VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);
		//vulkanFunctions.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &geomBuildInfo, ranges.data());
		//device->FlushCommandBuffer(commandBuffer);

		DestroyScratchBuffer(scratchBuffer);

		return accelerationStructure;
	}

	AccelerationStructure RayTracedSceneRenderer::CreateAccelerationStructure(VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
	{
		AccelerationStructure result{};

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VulkanAllocatorVolt allocator{};
		result.allocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, result.buffer);

		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = result.buffer;
		accelerationStructureCreateInfo.size = buildSizeInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = type;

		//auto device = GraphicsContextVolt::GetDevice();
		//const auto& vulkanFunctions = Renderer::GetVulkanFunctions();

		//vulkanFunctions.vkCreateAccelerationStructureKHR(device->GetHandle(), &accelerationStructureCreateInfo, nullptr, &result.handle);

		VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
		addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addrInfo.accelerationStructure = result.handle;

		//result.deviceAddress = vulkanFunctions.vkGetAccelerationStructureDeviceAddressKHR(device->GetHandle(), &addrInfo);
		return result;
	}

	void RayTracedSceneRenderer::DestroyAccelerationStructure(AccelerationStructure accelerationStructure)
	{
		//const auto& vulkanFunctions = Renderer::GetVulkanFunctions();

		VulkanAllocatorVolt allocator{};
		allocator.DestroyBuffer(accelerationStructure.buffer, accelerationStructure.allocation);
		//vulkanFunctions.vkDestroyAccelerationStructureKHR(GraphicsContextVolt::GetDevice()->GetHandle(), accelerationStructure.handle, nullptr);
	}

	ScratchBuffer RayTracedSceneRenderer::CreateScratchBuffer(VkDeviceSize deviceSize)
	{
		ScratchBuffer scratchBuffer{};

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = deviceSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VulkanAllocatorVolt allocator{};
		scratchBuffer.allocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, scratchBuffer.handle);

		VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
		bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAddressInfo.buffer = scratchBuffer.handle;

		//auto device = GraphicsContextVolt::GetDevice();
		//scratchBuffer.deviceAddress = vkGetBufferDeviceAddress(device->GetHandle(), &bufferDeviceAddressInfo);
		return scratchBuffer;
	}

	void RayTracedSceneRenderer::DestroyScratchBuffer(ScratchBuffer scratchBuffer)
	{
		VulkanAllocatorVolt allocator{};
		allocator.DestroyBuffer(scratchBuffer.handle, scratchBuffer.allocation);
	}
}
