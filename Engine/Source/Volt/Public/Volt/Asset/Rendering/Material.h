#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetFactory.h>

#include <CoreUtilities/Core.h>

#include <Mosaic/MosaicGraph.h>

#include <RHIModule/Pipelines/ComputePipeline.h>
#include <RHIModule/Images/SamplerState.h>

#include <string>

namespace Volt
{
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
		Material(RefPtr<RHI::ComputePipeline> computePipeline);
		~Material() override = default;

		void Compile();

		const std::string& GetName() const;
		bool HasFlag() const;
		void SetTexture(Ref<Texture2D> texture, uint32_t index);
		void RemoveTexture(uint32_t index);

		bool ClearAndGetIsDirty();

		Vector<AssetHandle> GetTextureHandles() const;

		inline const Vector<Ref<Texture2D>>& GetTextures() const { return m_textures; }
		inline const Vector<RefPtr<RHI::SamplerState>>& GetSamplers() const { return m_samplers; }

		inline RefPtr<RHI::ComputePipeline> GetComputePipeline() const { return m_computePipeline; } // #TODO_Ivar: Used for binding, should probably be done in a different way
	
		inline Mosaic::MosaicGraph& GetGraph() { return *m_graph; }
		inline const Mosaic::MosaicGraph& GetGraph() const { return *m_graph; }

		static AssetType GetStaticType() { return AssetTypes::Material; }
		AssetType GetType() override { return GetStaticType(); };
		void OnDependencyChanged(AssetHandle dependencyHandle, AssetChangedState state) override;

	private:
		friend class MosaicGraphImporter;
		friend class MaterialSerializer;

		Scope<Mosaic::MosaicGraph> m_graph;

		Vector<Ref<Texture2D>> m_textures;
		Vector<RefPtr<RHI::SamplerState>> m_samplers;

		RefPtr<RHI::ComputePipeline> m_computePipeline;

		bool m_isEngineMaterial = false;
		std::atomic_bool m_isDirty = false;
		VoltGUID m_materialGUID;
	};

	VT_REGISTER_ASSET_FACTORY(AssetTypes::Material, Material);
}
