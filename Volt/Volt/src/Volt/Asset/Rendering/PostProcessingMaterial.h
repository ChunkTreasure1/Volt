#pragma once
#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Rendering/Shader/Shader.h"

namespace Volt
{
	class Shader;
	class ComputePipeline;
	class CommandBuffer;
	class Image2D;

	class PostProcessingMaterial : public Asset
	{
	public:
		struct TextureInfo
		{
			Ref<Texture2D> texture;
			std::string name;
		};

		PostProcessingMaterial() = default;
		~PostProcessingMaterial() override;

		void Initialize(Ref<Shader> shader);

		void Invalidate();
		void SetShader(Ref<Shader> shader);

 		void Render(Ref<CommandBuffer> commandBuffer, Ref<Image2D> outputImage);
		const std::string& GetName() const;

		inline const std::unordered_map<uint32_t, TextureInfo>& GetTextures() const { return myTextures; }
		inline std::unordered_map<uint32_t, TextureInfo>& GetTexturesMutable() { return myTextures; }
		inline ShaderDataBuffer& GetMaterialData() { return myMaterialData; }

		template<typename T>
		void SetValue(const std::string& valueName, const T& value);

		static AssetType GetStaticType() { return AssetType::PostProcessingMaterial; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }
	
	private:
		std::unordered_map<uint32_t, TextureInfo> myTextures;

		friend class PostProcessingMaterialImporter;
		friend class PostProcessingMaterialSerializer;

		Ref<Shader> myShader;
		Ref<ComputePipeline> myPipeline;
		ShaderDataBuffer myMaterialData;
	};

	template<typename T>
	inline void PostProcessingMaterial::SetValue(const std::string& valueName, const T& value)
	{
		myMaterialData.SetValue(valueName, value);
	}
}
