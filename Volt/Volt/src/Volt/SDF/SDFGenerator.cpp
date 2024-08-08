#include "vtpch.h"
#include "SDFGenerator.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Rendering/Shader/ShaderMap.h"
#include "Volt/Math/AABB.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/Resources/RenderGraphTextureResource.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>

#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/Containers/SparseBrickMap.h>

#include <libacc/bvh_tree.h>

namespace Volt
{
	SDFGenerator::SDFGenerator()
	{
	}

	Vector<MeshSDF> SDFGenerator::Generate(Mesh& mesh)
	{
		Vector<MeshSDF> result(mesh.GetSubMeshes().size());

		for (uint32_t i = 0; const auto & subMesh : mesh.GetSubMeshes())
		{
			result[i] = GenerateForSubMesh(mesh, i, subMesh);
			i++;
		}

		return result;
	}

	constexpr float VOXEL_SIZE = 5.f; // One point equals 5cm
	constexpr float TRIANGLE_THICKNESS = 1.f; // 1 cm

	MeshSDF SDFGenerator::GenerateForSubMesh(Mesh& mesh, const uint32_t subMeshIndex, const SubMesh& subMesh)
	{
		const auto boundingBox = mesh.GetSubMeshBoundingBox(subMeshIndex);

		const glm::vec3 localMin = boundingBox.min;
		const glm::vec3 localMax = boundingBox.max;

		const float width = glm::max(glm::abs(localMax.x - localMin.x), TRIANGLE_THICKNESS);
		const float height = glm::max(glm::abs(localMax.y - localMin.y), TRIANGLE_THICKNESS);
		const float depth = glm::max(glm::abs(localMax.z - localMin.z), TRIANGLE_THICKNESS);

		const uint32_t pointCountWidth = static_cast<uint32_t>(glm::ceil(width / VOXEL_SIZE));
		const uint32_t pointCountHeight = static_cast<uint32_t>(glm::ceil(height / VOXEL_SIZE));
		const uint32_t pointCountDepth = static_cast<uint32_t>(glm::ceil(depth / VOXEL_SIZE));

		const std::string meshName = !mesh.assetName.empty() ? " - " + mesh.assetName : "";

		MeshSDF result;
		result.size = { pointCountWidth, pointCountHeight, pointCountDepth };
		result.min = localMin;
		result.max = localMax;

		// Create 3D grid of RESOLUTION * BRICK_SIZE size.
		constexpr float BRICK_CELL_SIZE = VOXEL_SIZE * static_cast<float>(BRICK_SIZE);

		const uint32_t gridSizeWidth = static_cast<uint32_t>(glm::ceil(width / BRICK_CELL_SIZE));
		const uint32_t gridSizeHeight = static_cast<uint32_t>(glm::ceil(height / BRICK_CELL_SIZE));
		const uint32_t gridSizeDepth = static_cast<uint32_t>(glm::ceil(depth / BRICK_CELL_SIZE));

		const auto& meshIndices = mesh.GetIndices();
		const auto& vertexPositions = mesh.GetVertexContainer().positions;

		Vector<SDFBrick> brickGrid;
		vt::map<size_t, uint32_t> pointToBrickIndex;

		constexpr uint32_t SAMPLE_COUNT = 4;
		constexpr float MAX_TRACE_DISTANCE = 100000.f;

		auto subMeshIndices = std::span<const uint32_t>(meshIndices.begin() + subMesh.indexStartOffset, meshIndices.begin() + subMesh.indexStartOffset + subMesh.indexCount);
		auto subMeshVertices = std::span<const glm::vec3>(vertexPositions.begin() + subMesh.vertexStartOffset, vertexPositions.begin() + subMesh.vertexStartOffset + subMesh.vertexCount);

		acc::BVHTree<uint32_t, glm::vec3> subMeshBVH(subMeshIndices, subMeshVertices);

		Vector<glm::vec3> sampleDirections;
		for (uint32_t sampleIndexX = 0; sampleIndexX < SAMPLE_COUNT; sampleIndexX++)
		{
			for (uint32_t sampleIndexY = 0; sampleIndexY < SAMPLE_COUNT; sampleIndexY++)
			{
				float sampleX = sampleIndexX / static_cast<float>(SAMPLE_COUNT - 1); // 0 -> 1
				float sampleY = sampleIndexY / static_cast<float>(SAMPLE_COUNT - 1) * 2.f - 1.f; // -1 -> 1

				const float phi = sampleX * glm::two_pi<float>();
				const float theta = glm::acos(sampleY);

				sampleDirections.emplace_back(Math::DirectionToVector({ phi, theta }));
			}
		}

		brickGrid.clear();
		pointToBrickIndex.clear();

		for (uint32_t z = 0; z < gridSizeDepth; ++z)
		{
			for (uint32_t y = 0; y < gridSizeHeight; ++y)
			{
				for (uint32_t x = 0; x < gridSizeWidth; ++x)
				{
					const glm::vec3 brickCellPos = (glm::vec3{ x, y, z } *BRICK_CELL_SIZE + BRICK_CELL_SIZE / 2.f) + localMin;
					const glm::vec3 brickMin = brickCellPos - BRICK_CELL_SIZE / 2.f;
					const glm::vec3 brickMax = brickCellPos + BRICK_CELL_SIZE / 2.f;

					AABB aabb{ brickMin, brickMax };

					size_t brickHash = Math::HashCombine(std::hash<uint32_t>()(x), std::hash<uint32_t>()(y));
					brickHash = Math::HashCombine(brickHash, std::hash<uint32_t>()(z));

					bool voxelIntersectsTriangle = false;

					for (uint32_t idx = subMesh.indexStartOffset; idx < subMesh.indexStartOffset + subMesh.indexCount; idx += 3)
					{
						const uint32_t idx0 = meshIndices.at(idx + 0) + subMesh.vertexStartOffset;
						const uint32_t idx1 = meshIndices.at(idx + 1) + subMesh.vertexStartOffset;
						const uint32_t idx2 = meshIndices.at(idx + 2) + subMesh.vertexStartOffset;

						const glm::vec3& v0 = vertexPositions.at(idx0);
						const glm::vec3& v1 = vertexPositions.at(idx1);
						const glm::vec3& v2 = vertexPositions.at(idx2);

						if (aabb.IntersectTriangle(v0, v1, v2))
						{
							voxelIntersectsTriangle = true;
							break;
						}
					}

					if (!voxelIntersectsTriangle)
					{
						continue;
					}

					uint32_t brickIndex = 0;

					if (!pointToBrickIndex.contains(brickHash))
					{
						brickIndex = static_cast<uint32_t>(brickGrid.size());
						pointToBrickIndex[brickHash] = brickIndex;
						brickGrid.emplace_back();

						std::fill(brickGrid.back().data, brickGrid.back().data + BRICK_SIZE * BRICK_SIZE * BRICK_SIZE, 100'000.f);
					}
					else
					{
						brickIndex = pointToBrickIndex.at(brickHash);
					}

					auto& brick = brickGrid[brickIndex];

					brick.hasData = true;
					brick.min = brickMin;
					brick.max = brickMax;

					for (uint32_t bz = 0; bz < BRICK_SIZE; ++bz)
					{
						for (uint32_t by = 0; by < BRICK_SIZE; ++by)
						{
							for (uint32_t bx = 0; bx < BRICK_SIZE; ++bx)
							{
								const glm::vec3 pointPos = brickMin + glm::vec3{ bx * VOXEL_SIZE, by * VOXEL_SIZE, bz * VOXEL_SIZE };
								const uint32_t voxelIndex = Math::Get1DIndexFrom3DCoord(bx, by, bz, BRICK_SIZE, BRICK_SIZE);

								float minDistance = std::numeric_limits<float>::infinity();
								minDistance = glm::distance(subMeshBVH.closest_point(pointPos, minDistance), pointPos);

								size_t backfaceHitCount = 0;
								size_t hitCount = 0;

								for (size_t sample = 0; sample < sampleDirections.size(); sample++)
								{
									acc::Ray<glm::vec3> ray{ pointPos, sampleDirections.at(sample), glm::epsilon<float>(), MAX_TRACE_DISTANCE };
									acc::BVHTree<uint32_t, glm::vec3>::Hit hit;

									if (subMeshBVH.intersect(ray, &hit))
									{
										const uint32_t idx0 = subMeshIndices[hit.idx * 3 + 0];
										const uint32_t idx1 = subMeshIndices[hit.idx * 3 + 1];
										const uint32_t idx2 = subMeshIndices[hit.idx * 3 + 2];

										const glm::vec3 v0 = subMeshVertices[idx0];
										const glm::vec3 v1 = subMeshVertices[idx1];
										const glm::vec3 v2 = subMeshVertices[idx2];
									
										const glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

										hitCount++;

										const bool backface = glm::dot(ray.dir, normal) > 0.f;
										if (backface)
										{
											backfaceHitCount++;
										}
									}
								}

								float sdf = minDistance;
								if (backfaceHitCount > sampleDirections.size() / 2 && hitCount != 0)
								{
									sdf *= -1.f;
								}

								brick.data[voxelIndex] = sdf;
							}
						}
					}
				}
			}
		}

		result.brickGrid = brickGrid;

		struct BrickInfo
		{
			glm::vec3 min;
			glm::vec3 max;
		};

		const uint32_t brickDataCount = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;
		const uint32_t brickDataSize = sizeof(float) * brickDataCount;

		Vector<float> brickData(brickGrid.size()* brickDataSize);
		Vector<BrickInfo> brickInfoData(brickGrid.size());

		uint32_t currentOffset = 0;
		for (uint32_t index = 0; const auto & brick : brickGrid)
		{
			memcpy(&brickData[currentOffset], brick.data, brickDataSize);
			currentOffset += brickDataCount;

			brickInfoData[index].min = brick.min;
			brickInfoData[index].max = brick.max;

			index++;
		}

		// Create brick texture
		{
			const uint32_t size = static_cast<uint32_t>(std::ceil(std::pow(static_cast<double>(brickGrid.size()), 1.0 / 3.0))) * BRICK_SIZE;
			result.size = size;

			RHI::ImageSpecification imageSpec{};
			imageSpec.width = size;
			imageSpec.height = size;
			imageSpec.depth = size;
			imageSpec.format = RHI::PixelFormat::R32_SFLOAT;
			imageSpec.usage = RHI::ImageUsage::Storage;
			imageSpec.imageType = RHI::ResourceType::Image3D;
			imageSpec.debugName = std::format("SDF Brick Texture{} - {}", meshName, subMeshIndex);

			BindlessResourceRef<RHI::Image3D> brickTexture = BindlessResource<RHI::Image3D>::CreateRef(imageSpec);
			result.sdfTexture = brickTexture;

			RefPtr<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
			RenderGraph renderGraph{ commandBuffer };

			RenderGraphBufferHandle dataBufferHandle = renderGraph.CreateBuffer(RGUtils::CreateBufferDesc<float>(brickData.size(), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Brick Data"));
			renderGraph.AddMappedBufferUpload(dataBufferHandle, brickData.data(), brickData.size() * sizeof(float), "Upload Brick Data");

			RenderGraphBufferHandle brickInfoBufferHandle = renderGraph.CreateBuffer(RGUtils::CreateBufferDesc<BrickInfo>(brickInfoData.size(), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Brick Info"));
			renderGraph.AddMappedBufferUpload(brickInfoBufferHandle, brickInfoData.data(), brickInfoData.size() * sizeof(BrickInfo), "Upload Brick Info");

			RenderGraphBufferHandle sdfBricksBufferHandle = renderGraph.CreateBuffer(RGUtils::CreateBufferDesc<GPUSDFBrick>(brickGrid.size(), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "SDF Bricks"));
			RenderGraphImage3DHandle textureHandle = renderGraph.AddExternalImage3D(brickTexture->GetResource());

			RGUtils::ClearImage(renderGraph, textureHandle, { 1000.f }, "Clear SDF Image");

			renderGraph.AddPass("Allocate Bricks",
			[&](RenderGraph::Builder& builder)
			{
				builder.ReadResource(dataBufferHandle);
				builder.ReadResource(brickInfoBufferHandle);
				builder.WriteResource(textureHandle);
				builder.WriteResource(sdfBricksBufferHandle);

				builder.SetHasSideEffect();
				builder.SetIsComputePass();
			},
			[=](RenderContext& context)
			{
				auto pipeline = ShaderMap::GetComputePipeline("MeshSDFAllocator");

				context.BindPipeline(pipeline);

				context.SetConstant("brickTexture"_sh, textureHandle);
				context.SetConstant("bricks"_sh, sdfBricksBufferHandle);
				context.SetConstant("brickData"_sh, dataBufferHandle);
				context.SetConstant("brickInfo"_sh, brickInfoBufferHandle);
				context.SetConstant("brickTextureSize"_sh, size);

				context.Dispatch(static_cast<uint32_t>(brickGrid.size()), 1, 1);
			});

			RefPtr<RHI::StorageBuffer> targetSDFBrickBuffer;
			renderGraph.QueueBufferExtraction(sdfBricksBufferHandle, targetSDFBrickBuffer);

			renderGraph.Compile();
			renderGraph.ExecuteImmediateAndWait();

			result.sdfBricksBuffer = targetSDFBrickBuffer;
		}

		return result;
	}
}
