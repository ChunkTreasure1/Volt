#pragma once

#include "Volt/Asset/Asset.h"

#include <CoreUtilities/Core.h>
#include <Mosaic/MosaicGraph.h>

#include <string>

namespace Volt
{
	namespace RHI
	{
		class ComputePipeline;
		class SamplerState;
	}

	enum class MaterialFlag : uint16_t
	{
		None = 0,
		Opaque = BIT(0),
		Transparent = BIT(1)
	};
	
	VT_SETUP_ENUM_CLASS_OPERATORS(MaterialFlag);

	class Texture2D;

	class Material : public Asset
	{
	public:
		Material();
		~Material() override = default;

		void Compile();

		const std::string& GetName() const;
		bool HasFlag() const;
		void SetTexture(Ref<Texture2D> texture, uint32_t index);
		void RemoveTexture(uint32_t index);

		inline const std::vector<Ref<Texture2D>>& GetTextures() const { return m_textures; }
		inline const std::vector<Ref<RHI::SamplerState>>& GetSamplers() const { return m_samplers; }

		inline Ref<RHI::ComputePipeline> GetComputePipeline() const { return m_computePipeline; } // #TODO_Ivar: Used for binding, should probably be done in a different way
	
		inline Mosaic::MosaicGraph& GetGraph() { return *m_graph; }
		inline const Mosaic::MosaicGraph& GetGraph() const { return *m_graph; }

		static Volt::AssetType GetStaticType() { return Volt::AssetType::Material; }
		Volt::AssetType GetType() override { return GetStaticType(); };

	private:
		friend class MosaicGraphImporter;

		Scope<Mosaic::MosaicGraph> m_graph;

		std::vector<Ref<Texture2D>> m_textures;
		std::vector<Ref<RHI::SamplerState>> m_samplers;

		Ref<RHI::ComputePipeline> m_computePipeline;

		bool m_isEngineMaterial = false;
		VoltGUID m_materialGUID;
	};
}
