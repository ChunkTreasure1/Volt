#include "vtpch.h"
#include "SDFGenerator.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/Shader/ShaderMap.h"

#include <CoreUtilities/Containers/Vector.h>

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

	constexpr float RESOLUTION = 1.f; // One point equals 1cm
	constexpr float TRIANGLE_THICKNESS = 1.f; // 1 cm

	inline float Dot2(const glm::vec3& v) { return glm::dot(v, v); }

	// From https://iquilezles.org/articles/triangledistance/
	inline float UDFTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& p)
	{
		glm::vec3 v21 = v2 - v1; glm::vec3 p1 = p - v1;
		glm::vec3 v32 = v3 - v2; glm::vec3 p2 = p - v2;
		glm::vec3 v13 = v1 - v3; glm::vec3 p3 = p - v3;

		glm::vec3 nor = glm::cross(v21, v13);

		return glm::sqrt(
			// Inside/outside test
			(glm::sign(glm::dot(glm::cross(v21, nor), p1)) +
			 glm::sign(glm::dot(glm::cross(v32, nor), p2)) +
			 glm::sign(glm::dot(glm::cross(v13, nor), p3)) < 2.f)
			?
			// 3 edges
			glm::min(glm::min(
			Dot2(v21 * glm::clamp(glm::dot(v21, p1) / Dot2(v21), 0.f, 1.f) - p1),
			Dot2(v32 * glm::clamp(glm::dot(v32, p2) / Dot2(v32), 0.f, 1.f) - p2)),
			Dot2(v13 * glm::clamp(glm::dot(v13, p3) / Dot2(v13), 0.f, 1.f) - p3))
			:
			//1 face
			glm::dot(nor, p1) * glm::dot(nor, p1) / Dot2(nor));
	}

	float PointToTriangleDistance(const glm::vec3& p, const glm::vec3& tv0, const glm::vec3& tv1, const glm::vec3& tv2)
	{
		glm::vec3 edge0 = tv1 - tv0;
		glm::vec3 edge1 = tv2 - tv0;
		glm::vec3 v0 = tv0 - p;

		float a = glm::dot(edge0, edge0);
		float b = glm::dot(edge0, edge1);
		float c = glm::dot(edge1, edge1);
		float d = glm::dot(edge0, v0);
		float e = glm::dot(edge1, v0);

		float det = a * c - b * b;
		float s = b * e - c * d;
		float t = b * d - a * e;

		if (s + t <= det)
		{
			if (s < 0.f)
			{
				if (t < 0.f)
				{
					if (d < 0.f)
					{
						s = glm::clamp(-d / a, 0.f, 1.f);
						t = 0.f;
					}
					else
					{
						s = 0.f;
						t = glm::clamp(-e / c, 0.f, 1.f);
					}
				}
				else
				{
					s = 0.f;
					t = glm::clamp(-e / c, 0.f, 1.f);
				}
			}
			else if (t < 0.f)
			{
				s = glm::clamp(-d / a, 0.f, 1.f);
			}
			else
			{
				float invDet = 1.f / det;
				s *= invDet;
				t *= invDet;
			}
		}
		else
		{
			if (s < 0.0f)
			{
				float tmp0 = b + d;
				float tmp1 = c + e;
				if (tmp1 > tmp0)
				{
					float numer = tmp1 - tmp0;
					float denom = a - 2 * b + c;
					s = glm::clamp(numer / denom, 0.0f, 1.0f);
					t = 1 - s;
				}
				else
				{
					t = glm::clamp(-e / c, 0.0f, 1.0f);
					s = 0.0f;
				}
			}
			else if (t < 0.0f)
			{
				if (a + d > b + e)
				{
					float numer = c + e - b - d;
					float denom = a - 2 * b + c;
					s = glm::clamp(numer / denom, 0.0f, 1.0f);
					t = 1 - s;
				}
				else
				{
					s = glm::clamp(-e / c, 0.0f, 1.0f);
					t = 0.0f;
				}
			}
			else
			{
				float numer = c + e - b - d;
				float denom = a - 2 * b + c;
				s = glm::clamp(numer / denom, 0.0f, 1.0f);
				t = 1 - s;
			}
		}

		glm::vec3 closestPoint = tv0 + s * edge0 + t * edge1;
		return glm::length(closestPoint - p);
	}

	inline float SignedDistance(const glm::vec3& p, const glm::vec3& tv0, const glm::vec3& tv1, const glm::vec3& tv2)
	{
		glm::vec3 normal = glm::normalize(glm::cross(tv1 - tv0, tv2 - tv0));
		float distance = PointToTriangleDistance(p, tv0, tv1, tv2);
		float sign = glm::dot(p - tv0, normal);
		return sign < 0.f ? -distance : distance;
	}

	inline uint32_t Get1DIndex(uint32_t x, uint32_t y, uint32_t z, uint32_t maxX, uint32_t maxY)
	{
		return (z * maxX * maxY) + (y * maxX) + x;
	}

	MeshSDF SDFGenerator::GenerateForSubMesh(Mesh& mesh, const uint32_t subMeshIndex, const SubMesh& subMesh)
	{
		const auto boundingBox = mesh.GetSubMeshBoundingBox(subMeshIndex);

		const glm::vec3 localMin = boundingBox.min;
		const glm::vec3 localMax = boundingBox.max;

		const float width = glm::max(glm::abs(localMax.x - localMin.x), TRIANGLE_THICKNESS);
		const float height = glm::max(glm::abs(localMax.y - localMin.y), TRIANGLE_THICKNESS);
		const float depth = glm::max(glm::abs(localMax.z - localMin.z), TRIANGLE_THICKNESS);

		const uint32_t pointCountWidth = static_cast<uint32_t>(glm::ceil(width / RESOLUTION));
		const uint32_t pointCountHeight = static_cast<uint32_t>(glm::ceil(height / RESOLUTION));
		const uint32_t pointCountDepth = static_cast<uint32_t>(glm::ceil(depth / RESOLUTION));

#if 1
		const std::string meshName = !mesh.assetName.empty() ? " - " + mesh.assetName : "";

		RHI::ImageSpecification imageSpec{};
		imageSpec.width = pointCountWidth;
		imageSpec.height = pointCountHeight;
		imageSpec.depth = pointCountDepth;
		imageSpec.format = RHI::PixelFormat::R32_SFLOAT;
		imageSpec.usage = RHI::ImageUsage::Storage;
		imageSpec.imageType = RHI::ResourceType::Image3D;
		imageSpec.debugName = std::format("SDF Texture{} - {}", meshName, subMeshIndex);

		BindlessResourceRef<RHI::Image3D> sdfTexture = BindlessResource<RHI::Image3D>::CreateRef(imageSpec);

		RefPtr<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
		RenderGraph renderGraph{ commandBuffer };

		RenderGraphResourceHandle sdfTextureHandle = renderGraph.AddExternalImage3D(sdfTexture->GetResource());

		struct PassData
		{
			RenderGraphResourceHandle indexBuffer;
			RenderGraphResourceHandle vertexPositionsBuffer;
		};

		RGUtils::ClearImage3D(renderGraph, sdfTextureHandle, { 100'000.f });

		renderGraph.AddPass<PassData>("Generate Mesh SDF Pass",
		[&](RenderGraph::Builder& builder, PassData& data) 
		{
			data.indexBuffer = builder.AddExternalBuffer(mesh.GetIndexBuffer()->GetResource());
			data.vertexPositionsBuffer = builder.AddExternalBuffer(mesh.GetVertexPositionsBuffer()->GetResource());

			builder.WriteResource(sdfTextureHandle);

			builder.ReadResource(data.indexBuffer);
			builder.ReadResource(data.vertexPositionsBuffer);

			builder.SetHasSideEffect();
			builder.SetIsComputePass();
		},
		[=](const PassData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateSDFFromMesh");

			context.BindPipeline(pipeline);

			context.SetConstant("outSDFTexture"_sh, resources.GetImage3D(sdfTextureHandle));
			context.SetConstant("vertexPositions"_sh, resources.GetBuffer(data.vertexPositionsBuffer));
			context.SetConstant("indexBuffer"_sh, resources.GetBuffer(data.indexBuffer));
			context.SetConstant("indexCount"_sh, subMesh.indexCount);
			context.SetConstant("indexStartOffset"_sh, subMesh.indexStartOffset);
			context.SetConstant("vertexStartOffset"_sh, subMesh.vertexStartOffset);
			context.SetConstant("bbMin"_sh, localMin);
			context.SetConstant("voxelSize"_sh, RESOLUTION);
			context.SetConstant("size"_sh, glm::uvec3{ pointCountWidth, pointCountHeight, pointCountDepth });

			constexpr uint32_t GROUP_SIZE = 8;
			context.Dispatch(Math::DivideRoundUp(pointCountWidth, GROUP_SIZE), Math::DivideRoundUp(pointCountHeight, GROUP_SIZE), Math::DivideRoundUp(pointCountDepth, GROUP_SIZE));
		});

		renderGraph.Compile();
		renderGraph.ExecuteImmediateAndWait();

		MeshSDF result;
		result.sdfTexture = sdfTexture;
		result.size = { pointCountWidth, pointCountHeight, pointCountDepth };
		result.min = localMin;
		result.max = localMax;

		return result;

#elif 0
		Vector<float> sdfMap(pointCountWidth * pointCountHeight * pointCountDepth, 100'000.f);

		const auto& meshIndices = mesh.GetIndices();
		const auto& vertexPositions = mesh.GetVertexContainer().positions;

		for (uint32_t z = 0; z < pointCountDepth; ++z)
		{
			for (uint32_t y = 0; y < pointCountHeight; ++y)
			{
				for (uint32_t x = 0; x < pointCountWidth; ++x)
				{
					const glm::vec3 pointPos = glm::vec3{ x * RESOLUTION, y * RESOLUTION, z * RESOLUTION } + localMin;

					for (uint32_t idx = subMesh.indexStartOffset; idx < subMesh.indexStartOffset + subMesh.indexCount; idx += 3)
					{
						const uint32_t idx0 = meshIndices.at(idx + 0) + subMesh.vertexStartOffset;
						const uint32_t idx1 = meshIndices.at(idx + 1) + subMesh.vertexStartOffset;
						const uint32_t idx2 = meshIndices.at(idx + 2) + subMesh.vertexStartOffset;
						
						const glm::vec3& v0 = vertexPositions.at(idx0);
						const glm::vec3& v1 = vertexPositions.at(idx1);
						const glm::vec3& v2 = vertexPositions.at(idx2);
					
						const uint32_t index = Get1DIndex(x, y, z, pointCountWidth, pointCountHeight);
						const float distance = SignedDistance(pointPos, v0, v1, v2);

						if (std::abs(distance) < std::abs(sdfMap[index]))
						{
							sdfMap[index] = distance;
						}
					}
				}
			}
		}

		MeshSDF result;
		result.sdf = sdfMap;
		result.size = { pointCountWidth, pointCountHeight, pointCountDepth };
		result.min = localMin;
		result.max = localMax;

		return result;
#endif
	}
}
