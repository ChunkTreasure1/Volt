#include "sbpch.h"
#include "PostProcessingMaterialPanel.h"

#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Rendering/PostProcessingMaterial.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Rendering/RenderPipeline/ShaderRegistry.h>
#include <Volt/Rendering/Renderer.h>

PostProcessingMaterialPanel::PostProcessingMaterialPanel()
	: EditorWindow("Post Processing Material Editor")
{
}

void PostProcessingMaterialPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	myCurrentMaterial = std::reinterpret_pointer_cast<Volt::PostProcessingMaterial>(asset);
}

void PostProcessingMaterialPanel::UpdateMainContent()
{
	if (ImGui::Button("Save") && myCurrentMaterial)
	{
		Volt::AssetManager::Get().SaveAsset(myCurrentMaterial);
	}

	if (!myCurrentMaterial)
	{
		return;
	}

	UI::Header("Shader");
	std::vector<std::string> shaderNames;
	shaderNames.emplace_back("None");
	for (const auto& [name, shader] : Volt::ShaderRegistry::GetShaderRegistry())
	{
		if (!shader->IsInternal())
		{
			shaderNames.emplace_back(shader->GetName());
		}
	}

	int32_t selectedShader = 0;
	const std::string shaderName = myCurrentMaterial->GetName();

	auto it = std::find(shaderNames.begin(), shaderNames.end(), shaderName);
	if (it != shaderNames.end())
	{
		selectedShader = (int32_t)std::distance(shaderNames.begin(), it);
	}

	UI::PushId();
	if (UI::BeginProperties("Shader"))
	{
		if (UI::ComboProperty("Shader", selectedShader, shaderNames))
		{
			auto newPipeline = Volt::ShaderRegistry::GetShader(shaderNames.at(selectedShader));
			if (newPipeline)
			{
				myCurrentMaterial->SetShader(newPipeline);
			}
		}
		UI::EndProperties();
	}
	UI::PopId();

	UI::Header("Textures");

	UI::PushId();
	if (UI::BeginProperties("Textures"))
	{
		auto& textures = myCurrentMaterial->GetTexturesMutable();

		for (auto& [binding, textureInfo] : textures)
		{
			Volt::AssetHandle texHandle = Volt::Asset::Null();
			if (textureInfo.texture != Volt::Renderer::GetDefaultData().whiteTexture)
			{
				texHandle = textureInfo.texture->handle;
			}

			if (EditorUtils::Property(textureInfo.name, texHandle, Volt::AssetType::Texture))
			{
				Ref<Volt::Texture2D> newTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>(texHandle);
				if (newTexture && newTexture->IsValid())
				{
					textureInfo.texture = newTexture;
				}
				else
				{
					textureInfo.texture = Volt::Renderer::GetDefaultData().whiteTexture;
				}
			}
		}

		UI::EndProperties();
	}
	UI::PopId();

	if (myCurrentMaterial->GetMaterialData().IsValid())
	{
		ImGui::Separator();

		UI::Header("Material Parameters");

		UI::PushId();
		if (UI::BeginProperties("materialParams"))
		{
			auto& materialSpecializationParams = myCurrentMaterial->GetMaterialData();

			for (auto& [name, memberData] : materialSpecializationParams.GetMembers())
			{
				switch (memberData.type)
				{
					case Volt::ShaderUniformType::Bool: UI::Property(name, materialSpecializationParams.GetValue<bool>(name)); break;
					case Volt::ShaderUniformType::UInt:  UI::Property(name, materialSpecializationParams.GetValue<uint32_t>(name)); break;
					case Volt::ShaderUniformType::UInt2: UI::Property(name, materialSpecializationParams.GetValue<gem::vec2ui>(name)); break;
					case Volt::ShaderUniformType::UInt3: UI::Property(name, materialSpecializationParams.GetValue<gem::vec3ui>(name)); break;
					case Volt::ShaderUniformType::UInt4: UI::Property(name, materialSpecializationParams.GetValue<gem::vec4ui>(name)); break;

					case Volt::ShaderUniformType::Int: UI::Property(name, materialSpecializationParams.GetValue<int32_t>(name)); break;
					case Volt::ShaderUniformType::Int2: UI::Property(name, materialSpecializationParams.GetValue<gem::vec2i>(name)); break;
					case Volt::ShaderUniformType::Int3: UI::Property(name, materialSpecializationParams.GetValue<gem::vec3i>(name)); break;
					case Volt::ShaderUniformType::Int4: UI::Property(name, materialSpecializationParams.GetValue<gem::vec4i>(name)); break;

					case Volt::ShaderUniformType::Float: UI::Property(name, materialSpecializationParams.GetValue<float>(name)); break;
					case Volt::ShaderUniformType::Float2: UI::Property(name, materialSpecializationParams.GetValue<gem::vec2>(name)); break;
					case Volt::ShaderUniformType::Float3: UI::Property(name, materialSpecializationParams.GetValue<gem::vec3>(name)); break;
					case Volt::ShaderUniformType::Float4: UI::Property(name, materialSpecializationParams.GetValue<gem::vec4>(name)); break;
				}
			}

			UI::EndProperties();
		}
		UI::PopId();
	}
}

void PostProcessingMaterialPanel::OnClose()
{
	myCurrentMaterial = nullptr;
}