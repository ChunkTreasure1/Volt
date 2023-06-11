#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"

#include <string>

namespace Volt
{
	class RenderPipeline;
	class Texture2D;
	class Image2D;
	class CommandBuffer;

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

	struct MaterialData
	{
		glm::vec4 color = 1.f;
		glm::vec3 emissiveColor = 1.f;
		float emissiveStrength = 0.f;

		float roughness = 0.5f;
		float metalness = 0.f;
		float normalStrength = 0.f;
	};

	class SubMaterial
	{
	public:
		SubMaterial();
		SubMaterial(const SubMaterial& subMaterial);
		SubMaterial(const std::string& aName, uint32_t aIndex, Ref<Shader> shader);
		SubMaterial(const std::string& aName, uint32_t aIndex, const RenderPipelineSpecification& specification);
		~SubMaterial();

		void Bind(Ref<CommandBuffer> commandBuffer);
		void PushMaterialData(Ref<CommandBuffer> commandBuffer) const;

		void SetShader(Ref<Shader> shader);
		void SetTexture(const std::string& name, Ref<Texture2D> texture);

		void Invalidate();
		void InvalidatePipeline();

		void RecompilePermutation();

		inline const Ref<RenderPipeline> GetPipeline() const { return myPipeline; }
		inline void SetName(const std::string& name) { myName = name; }

		inline void SetFlags(MaterialFlag flags) { myMaterialFlags = flags; GenerateHash(); }
		inline bool HasFlag(MaterialFlag flag) { return (myMaterialFlags & flag) != MaterialFlag::All; }
		void SetFlag(MaterialFlag flag, bool state);

		inline void SetTopology(const Topology topology) { myTopology = topology; InvalidatePipeline(myPipeline->GetSpecification().shader); }
		inline void SetCullMode(const CullMode cullMode) { myCullMode = cullMode; InvalidatePipeline(myPipeline->GetSpecification().shader); }
		inline void SetFillMode(const FillMode fillMode) { myTriangleFillMode = fillMode; InvalidatePipeline(myPipeline->GetSpecification().shader); }
		inline void SetDepthMode(const DepthMode depthMode) { myDepthMode = depthMode; InvalidatePipeline(myPipeline->GetSpecification().shader); }

		inline const Topology GetTopology() const { return myTopology; }
		inline const CullMode GetCullMode() const { return myCullMode; }
		inline const FillMode GetFillMode() const { return myTriangleFillMode; }
		inline const DepthMode GetDepthMode() const { return myDepthMode; }

		inline const std::string& GetName() const { return myName; }
		inline const std::map<std::string, Ref<Texture2D>>& GetTextures() const { return myTextures; }

		inline const MaterialFlag GetFlags() const { return myMaterialFlags; }
		inline const MaterialData& GetMaterialData() const { return myMaterialData; }

		inline MaterialData& GetMaterialData() { return myMaterialData; }
		inline ShaderDataBuffer& GetMaterialSpecializationData() { return myMaterialSpecializationData; }
		inline std::map<ShaderStage, ShaderDataBuffer>& GetPipelineGenerationDatas() { return myPipelineGenerationData; }

		bool operator==(const SubMaterial& rhs);
		bool operator!=(const SubMaterial& rhs);

		friend bool operator>(const SubMaterial& lhs, const SubMaterial& rhs);
		friend bool operator<(const SubMaterial& lhs, const SubMaterial& rhs);

		static Ref<SubMaterial> Create(const std::string& aName, uint32_t aIndex, Ref<Shader> aPipeline);
		static Ref<SubMaterial> Create(const std::string& aName, uint32_t aIndex, const RenderPipelineSpecification& specification);
		static Ref<SubMaterial> Create();

		void Set(uint32_t binding, Ref<Image2D> image);

		template<typename T>
		void SetValue(const std::string& valueName, const T& value);

		template<typename T>
		const T GetValue(const std::string& valueName);

	private:
		friend class MaterialImporter;

		void SetupMaterialFromPipeline();
		void InvalidateDescriptorSets();
		void UpdateDescriptorSetsForRendering(Ref<CommandBuffer> commandBuffer);
		void GenerateHash();
		void Release();

		void InvalidatePipeline(Ref<Shader> shader);
		void InvalidatePipeline(const RenderPipelineSpecification& specification);

		const std::vector<FramebufferAttachment> GetAttachmentsFromMaterialFlags() const;

		Ref<RenderPipeline> myPipeline;

		std::map<std::string, Ref<Texture2D>> myTextures;

		std::map<uint32_t, Ref<Image2D>> myMaterialImages;
		std::vector<VkWriteDescriptorSet> myMaterialWriteDescriptors;
		std::vector<VkDescriptorSet> myMaterialDescriptorSets;
		std::vector<VkDescriptorPool> myMaterialDescriptorPools;
		std::vector<bool> myDirtyDescriptorSets;

		MaterialFlag myMaterialFlags = MaterialFlag::Deferred | MaterialFlag::CastAO | MaterialFlag::CastShadows;
		
		// Render pipeline info
		Topology myTopology = Topology::TriangleList;
		CullMode myCullMode = CullMode::Back;
		FillMode myTriangleFillMode = FillMode::Solid;
		DepthMode myDepthMode = DepthMode::ReadWrite;

		MaterialData myMaterialData;
		ShaderDataBuffer myMaterialSpecializationData;
		std::map<ShaderStage, ShaderDataBuffer> myPipelineGenerationData;

		std::string myName;
		uint32_t myIndex = 0;
		size_t myHash = 0;
	};

	template<typename T>
	inline void SubMaterial::SetValue(const std::string& valueName, const T& value)
	{
		myMaterialSpecializationData.SetValue(valueName, value);
	}

	template<typename T>
	inline const T SubMaterial::GetValue(const std::string& valueName)
	{
		return myMaterialSpecializationData.GetValue<T>(valueName);
	}
}
