#include "vtpch.h"
#include "SubMaterial.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	SubMaterial::SubMaterial()
	{
	}

	SubMaterial::SubMaterial(const SubMaterial& subMaterial)
	{
		m_name = subMaterial.m_name;
		m_index = subMaterial.m_index;
		m_topology = subMaterial.m_topology;
		m_cullMode = subMaterial.m_cullMode;
		m_triangleFillMode = subMaterial.m_triangleFillMode;
		m_depthMode = subMaterial.m_depthMode;
		m_materialFlags = subMaterial.m_materialFlags;

		InvalidatePipeline(subMaterial.m_pipeline->GetShader());
		Invalidate();
		GenerateHash();

		m_textures = subMaterial.m_textures;
	}

	SubMaterial::SubMaterial(const std::string& aName, uint32_t aIndex, Ref<RHI::Shader> shader)
		: m_name(aName), m_index(aIndex)
	{
		InvalidatePipeline(shader);
		Invalidate();
	}

	SubMaterial::~SubMaterial()
	{
	}

	void SubMaterial::SetShader(Ref<RHI::Shader> shader)
	{
		InvalidatePipeline(shader);
		Invalidate();
	}

	void SubMaterial::AddTexture(Ref<Texture2D> texture)
	{
		m_textures.emplace_back(texture);
	}

	void SubMaterial::SetFlag(MaterialFlag flag, bool state)
	{
		if (state)
		{
			m_materialFlags = m_materialFlags | flag;
		}
		else
		{
			m_materialFlags = m_materialFlags & ~(flag);
		}

		GenerateHash();
	}

	bool SubMaterial::operator==(const SubMaterial& rhs)
	{
		return m_hash == rhs.m_hash;
	}

	bool SubMaterial::operator!=(const SubMaterial& rhs)
	{
		return m_hash != rhs.m_hash;
	}

	void SubMaterial::Invalidate()
	{
		/*const auto originalTextures = myTextures;
		myTextures.clear();

		for (const auto& descriptorPool : myMaterialDescriptorPools)
		{
			vkDestroyDescriptorPool(GraphicsContextVolt::GetDevice()->GetHandle(), descriptorPool, nullptr);
		}

		myMaterialDescriptorPools.clear();
		myMaterialDescriptorSets.clear();

		SetupMaterialFromPipeline();

		for (auto& [binding, texture] : myTextures)
		{
			auto it = originalTextures.find(binding);
			if (it != originalTextures.end())
			{
				texture = it->second;
			}
		}

		GenerateHash();
		Renderer::UpdateMaterial(this);*/
	}

	void SubMaterial::InvalidatePipeline()
	{
		InvalidatePipeline(m_pipeline->GetShader());
	}

	Ref<SubMaterial> SubMaterial::Create(const std::string& aName, uint32_t aIndex, Ref<RHI::Shader> aShader)
	{
		return CreateRef<SubMaterial>(aName, aIndex, aShader);
	}

	Ref<SubMaterial> SubMaterial::Create()
	{
		return CreateRef<SubMaterial>();
	}

	void SubMaterial::GenerateHash()
	{
		m_hash = 0;
		//myHash = Utility::HashCombine(myHash, myPipeline->GetHash());
		//myHash = Utility::HashCombine(myHash, std::hash<std::string>()(myName));
		//myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()(myIndex));
		//myHash = Utility::HashCombine(myHash, std::hash<size_t>()(myTextures.size()));
		//myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myMaterialFlags));
		//myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myTopology));
		//myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myCullMode));
		//myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myTriangleFillMode));
		//myHash = Utility::HashCombine(myHash, std::hash<uint32_t>()((uint32_t)myDepthMode));
	}

	void SubMaterial::InvalidatePipeline(Ref<RHI::Shader> shader)
	{
		RHI::RenderPipelineCreateInfo info{};
		info.shader = shader;
		info.topology = m_topology;
		info.cullMode = m_cullMode;
		info.fillMode = m_triangleFillMode;
		info.depthMode = m_depthMode;
		info.name = m_name;

		m_pipeline = RHI::RenderPipeline::Create(info);
		GenerateHash();
	}

	bool operator>(const SubMaterial& lhs, const SubMaterial& rhs)
	{
		return lhs.m_hash > rhs.m_hash;
	}

	bool operator<(const SubMaterial& lhs, const SubMaterial& rhs)
	{
		return lhs.m_hash < rhs.m_hash;
	}
}
