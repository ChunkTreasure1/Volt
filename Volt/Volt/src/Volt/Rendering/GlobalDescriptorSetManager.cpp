#include "vtpch.h"
#include "GlobalDescriptorSetManager.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Texture/TextureTable.h"

namespace Volt
{
	void GlobalDescriptorSetManager::Initialize()
	{
		// Renderer Buffers
		{
			GlobalDescriptorSetSpecification spec{};
			spec.set = Sets::RENDERER_BUFFERS;
			spec.updateAfterBind = false;
			spec.descriptorSetCount = Renderer::GetFramesInFlightCount();
			spec.descriptorBindings =
			{
				{ Bindings::CAMERA_BUFFER, 1, DescriptorType::UniformBuffer, ShaderStage::All },
				{ Bindings::INDIRECT_ARGS, 1, DescriptorType::StorageBuffer, ShaderStage::Compute },
				{ Bindings::OBJECT_DATA, 1, DescriptorType::StorageBuffer, ShaderStage::Vertex | ShaderStage::Compute | ShaderStage::Pixel },
				{ Bindings::DIRECTIONAL_LIGHT, 1, DescriptorType::UniformBuffer, ShaderStage::Compute | ShaderStage::Pixel | ShaderStage::Geometry },
				{ Bindings::SCENE_DATA, 1, DescriptorType::UniformBuffer, ShaderStage::Vertex | ShaderStage::Compute | ShaderStage::Pixel },
				{ Bindings::POINT_LIGHTS, 1, DescriptorType::StorageBuffer, ShaderStage::Compute | ShaderStage::Pixel | ShaderStage::Geometry },
				{ Bindings::SPOT_LIGHTS, 1, DescriptorType::StorageBuffer, ShaderStage::Compute | ShaderStage::Pixel | ShaderStage::Vertex },
				{ Bindings::ANIMATION_DATA, 1, DescriptorType::StorageBuffer, ShaderStage::Vertex },
				{ Bindings::SPHERE_LIGHTS, 1, DescriptorType::StorageBuffer, ShaderStage::Compute | ShaderStage::Pixel },
				{ Bindings::RECTANGLE_LIGHTS, 1, DescriptorType::StorageBuffer, ShaderStage::Compute | ShaderStage::Pixel },
				{ Bindings::PARTICLE_INFO, 1, DescriptorType::StorageBuffer, ShaderStage::Pixel | ShaderStage::Vertex | ShaderStage::Geometry },
				{ Bindings::VISIBLE_POINT_LIGHTS, 1, DescriptorType::StorageBuffer, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::RENDERER_DATA, 1, DescriptorType::UniformBuffer, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::PAINTED_VERTEX_COLORS, 1, DescriptorType::StorageBuffer, ShaderStage::Vertex | ShaderStage::Compute },
				{ Bindings::VISIBLE_SPOT_LIGHTS, 1, DescriptorType::StorageBuffer, ShaderStage::Pixel | ShaderStage::Compute },
			};

			myDescriptorSets[Sets::RENDERER_BUFFERS] = GlobalDescriptorSet::Create(spec);
		}

		// Main Buffers
		{
			GlobalDescriptorSetSpecification spec{};
			spec.set = Sets::MAINBUFFERS;
			spec.updateAfterBind = false;
			spec.descriptorSetCount = Renderer::GetFramesInFlightCount();
			spec.descriptorBindings =
			{
				{ Bindings::MATERIAL_TABLE, 1, DescriptorType::StorageBuffer, ShaderStage::All }
			};

			myDescriptorSets[Sets::MAINBUFFERS] = GlobalDescriptorSet::Create(spec);
		}

		// Textures
		{
			GlobalDescriptorSetSpecification spec{};
			spec.set = Sets::TEXTURES;
			spec.updateAfterBind = true;
			spec.descriptorSetCount = Renderer::GetFramesInFlightCount();
			spec.descriptorBindings =
			{
				{ Bindings::TEXTURE2DTABLE, TextureTable::TEXTURE2D_COUNT, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::TEXTURECUBETABLE, TextureTable::TEXTURECUBE_COUNT, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::TEXTURE3DTABLE, TextureTable::TEXTURE3D_COUNT, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex }
			};

			myDescriptorSets[Sets::TEXTURES] = GlobalDescriptorSet::Create(spec);
		}

		// Sampler states
		{
			GlobalDescriptorSetSpecification spec{};
			spec.set = Sets::SAMPLERS;
			spec.updateAfterBind = false;
			spec.descriptorSetCount = Renderer::GetFramesInFlightCount();
			spec.descriptorBindings =
			{
				{ Bindings::SAMPLER_LINEAR, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::SAMPLER_LINEAR_POINT, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex},
				{ Bindings::SAMPLER_POINT, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::SAMPLER_POINT_LINEAR, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },

				{ Bindings::SAMPLER_LINEAR_CLAMP, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::SAMPLER_LINEAR_POINT_CLAMP, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::SAMPLER_POINT_CLAMP, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::SAMPLER_POINT_LINEAR_CLAMP, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },

				{ Bindings::SAMPLER_ANISO, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::SAMPLER_SHADOW, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
				{ Bindings::SAMPLER_REDUCE, 1, DescriptorType::Sampler, ShaderStage::Pixel | ShaderStage::Compute | ShaderStage::Vertex },
			};

			myDescriptorSets[Sets::SAMPLERS] = GlobalDescriptorSet::Create(spec);
		}

		// PBR Resources
		{
			constexpr uint32_t MAX_SPOT_LIGHT_SHADOWS = 8;
			constexpr uint32_t MAX_POINT_LIGHT_SHADOWS = 8;

			GlobalDescriptorSetSpecification spec{};
			spec.set = Sets::PBR_RESOURCES;
			spec.updateAfterBind = true;
			spec.descriptorSetCount = Renderer::GetFramesInFlightCount();
			spec.descriptorBindings =
			{
				{ Bindings::BRDF, 1, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::IRRADIANCE, 1, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::RADIANCE, 1, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::DIR_SHADOWMAP, 1, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::SPOT_SHADOWMAPS, MAX_SPOT_LIGHT_SHADOWS, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::POINT_SHADOWMAPS, MAX_POINT_LIGHT_SHADOWS , DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::VOLUMETRIC_FOG_TEXTURE, 1, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute },
				{ Bindings::AO_TEXTURE, 1, DescriptorType::Texture, ShaderStage::Pixel | ShaderStage::Compute }
			};

			myDescriptorSets[Sets::PBR_RESOURCES] = GlobalDescriptorSet::Create(spec);
		}

		// Draw Buffers
		{
			GlobalDescriptorSetSpecification spec{};
			spec.set = Sets::DRAW_BUFFERS;
			spec.updateAfterBind = false;
			spec.descriptorSetCount = Renderer::GetFramesInFlightCount();
			spec.descriptorBindings =
			{
				{ Bindings::INDIRECT_COUNTS, 1, DescriptorType::StorageBuffer, ShaderStage::Compute },
				{ Bindings::DRAW_TO_OBJECT_ID, 1, DescriptorType::StorageBuffer, ShaderStage::Vertex | ShaderStage::Compute | ShaderStage::Pixel },
			};

			myDescriptorSets[Sets::DRAW_BUFFERS] = GlobalDescriptorSet::Create(spec);
		}
	}

	void GlobalDescriptorSetManager::Shutdown()
	{
		myDescriptorSets.clear();
	}

	Ref<GlobalDescriptorSet> GlobalDescriptorSetManager::CreateCopyOfType(uint32_t type)
	{
		return CreateRef<GlobalDescriptorSet>(myDescriptorSets.at(type)->GetSpecification());
	}

	std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>> GlobalDescriptorSetManager::CreateFullCopy()
	{
		std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>> result;
		for (const auto& [type, descriptorSet] : myDescriptorSets)
		{
			result[type] = CreateCopyOfType(type);
		}

		return result;
	}

	Ref<GlobalDescriptorSet> GlobalDescriptorSetManager::GetDescriptorSet(uint32_t set)
	{
		return myDescriptorSets.at(set);
	}

	bool GlobalDescriptorSetManager::HasDescriptorSet(uint32_t set)
	{
		if (myDescriptorSets.contains(set))
		{
			return true;
		}

		return false;
	}
}
