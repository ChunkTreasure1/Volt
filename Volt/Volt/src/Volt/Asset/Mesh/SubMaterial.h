#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include <VoltRHI/Pipelines/RenderPipeline.h>

#include <string>

namespace Volt
{
	class Texture2D;

	enum class MaterialFlag : uint32_t
	{
		All = 0,
		None = BIT(0),
		Opaque = BIT(2),
		Transparent = BIT(3),

		CastShadows = BIT(4),
		CastAO = BIT(5),

		SSS = BIT(6),
		Deferred = BIT(7),
		Decal = BIT(8)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(MaterialFlag);

	class SubMaterial
	{
	public:
		SubMaterial();
		SubMaterial(const SubMaterial& subMaterial);
		SubMaterial(const std::string& aName, uint32_t aIndex, Ref<RHI::Shader> shader);
		~SubMaterial();

		void Invalidate();
		void InvalidatePipeline();

		inline const Ref<RHI::RenderPipeline> GetPipeline() const { return m_pipeline; }
		inline void SetName(const std::string& name) { m_name = name; }

		inline void SetFlags(MaterialFlag flags) { m_materialFlags = flags; GenerateHash(); }
		inline bool HasFlag(MaterialFlag flag) { return (m_materialFlags & flag) != MaterialFlag::All; }
		void SetFlag(MaterialFlag flag, bool state);
		void SetShader(Ref<RHI::Shader> shader);

		inline void SetTopology(const RHI::Topology topology) { m_topology = topology; InvalidatePipeline(m_pipeline->GetShader()); }
		inline void SetCullMode(const RHI::CullMode cullMode) { m_cullMode = cullMode; InvalidatePipeline(m_pipeline->GetShader()); }
		inline void SetFillMode(const RHI::FillMode fillMode) { m_triangleFillMode = fillMode; InvalidatePipeline(m_pipeline->GetShader()); }
		inline void SetDepthMode(const RHI::DepthMode depthMode) { m_depthMode = depthMode; InvalidatePipeline(m_pipeline->GetShader()); }

		inline const RHI::Topology GetTopology() const { return m_topology; }
		inline const RHI::CullMode GetCullMode() const { return m_cullMode; }
		inline const RHI::FillMode GetFillMode() const { return m_triangleFillMode; }
		inline const RHI::DepthMode GetDepthMode() const { return m_depthMode; }

		inline const std::string& GetName() const { return m_name; }
		inline const std::map<std::string, Ref<Texture2D>>& GetTextures() const { return m_textures; }

		inline const MaterialFlag GetFlags() const { return m_materialFlags; }

		bool operator==(const SubMaterial& rhs);
		bool operator!=(const SubMaterial& rhs);

		friend bool operator>(const SubMaterial& lhs, const SubMaterial& rhs);
		friend bool operator<(const SubMaterial& lhs, const SubMaterial& rhs);

		static Ref<SubMaterial> Create(const std::string& aName, uint32_t aIndex, Ref<RHI::Shader> aPipeline);
		static Ref<SubMaterial> Create();

	private:
		friend class MaterialImporter;

		void GenerateHash();
		void InvalidatePipeline(Ref<RHI::Shader> shader);

		Ref<RHI::RenderPipeline> m_pipeline;
		std::map<std::string, Ref<Texture2D>> m_textures;

		MaterialFlag m_materialFlags = MaterialFlag::Deferred | MaterialFlag::CastAO | MaterialFlag::CastShadows;
		
		// Render pipeline info
		RHI::Topology m_topology = RHI::Topology::TriangleList;
		RHI::CullMode m_cullMode = RHI::CullMode::Back;
		RHI::FillMode m_triangleFillMode = RHI::FillMode::Solid;
		RHI::DepthMode m_depthMode = RHI::DepthMode::ReadWrite;

		std::string m_name;
		uint32_t m_index = 0;
		size_t m_hash = 0;
	};
}
