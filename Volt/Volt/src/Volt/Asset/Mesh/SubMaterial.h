#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include "Volt/Rendering/Shader/Shader.h"

#include <string>

namespace Volt
{
	class Texture2D;
	class ConstantBuffer;
	class SubMaterial
	{
	public:
		SubMaterial() = default;
		SubMaterial(const std::string& aName, uint32_t aIndex, Ref<Shader> aShader);
		~SubMaterial();

		void Bind(bool aBindShader = true);
		void SetTexture(uint32_t binding, Ref<Texture2D> texture);
		void Invalidate();
		void UpdateBuffer(bool deferr = false);
		void SetShader(Ref<Shader> aShader);

		inline const size_t GetShaderHash() const { return myShader->GetHash(); }
		inline const Ref<Shader> GetShader() const { return myShader; }

		inline const std::string& GetName() const { return myName; }
		inline const Shader::ShaderResources& GetResources() const { return myShaderResources; }
		inline const std::map<uint32_t, Ref<Texture2D>>& GetTextures() const { return myTextures; }

		bool operator==(const SubMaterial& rhs);
		bool operator!=(const SubMaterial& rhs);

		friend bool operator>(const SubMaterial& lhs, const SubMaterial& rhs);
		friend bool operator<(const SubMaterial& lhs, const SubMaterial& rhs);

		template<typename T>
		bool SetParameter(const std::string& paramName, const T& value);

		template<typename T>
		const T GetParameter(const std::string& paramName);

		static Ref<SubMaterial> Create(const std::string& aName, uint32_t aIndex, Ref<Shader> aShader);
		static Ref<SubMaterial> Create();

	private:
		friend class MaterialImporter;

		void SetupMaterialFromShader();

		std::map<uint32_t, Ref<Texture2D>> myTextures;

		Ref<Shader> myShader;
		Ref<ConstantBuffer> myMaterialBuffer;

		std::string myName;
		Shader::ShaderResources myShaderResources;
		uint32_t myIndex = 0;

		size_t myHash = 0;
		bool myMaterialBufferDirty = false;
	};

	template<typename T>
	inline bool SubMaterial::SetParameter(const std::string& paramName, const T& value)
	{
		if (!myShaderResources.materialBuffer.exists)
		{
			VT_CORE_WARN("Trying to set a parameter in material {0} with shader {1}. But it does not have a material buffer!", myName, myShader->GetName());
			return false;
		}

		if (myShaderResources.materialBuffer.parameters.find(paramName) == myShaderResources.materialBuffer.parameters.end())
		{
			VT_CORE_WARN("Trying to set parameter {0} in material {1} with shader {2}. However a parameter with that name does not exist!", paramName, myName, myShader->GetName());
			return false;
		}

		// #TODO(Ivar): Add way to make sure the type is correct
		const auto& param = myShaderResources.materialBuffer.parameters.at(paramName);

		(*(T*)&myShaderResources.materialBuffer.data[param.offset]) = value;
		myMaterialBufferDirty = true;
		return true;
	}

	template<typename T>
	inline const T SubMaterial::GetParameter(const std::string& paramName)
	{
		if (!myShaderResources.materialBuffer.exists)
		{
			VT_CORE_WARN("Trying to get a parameter in material {0} with shader {1}. But it does not have a material buffer!", myName, myShader->GetName());
			return T{};
		}

		if (myShaderResources.materialBuffer.parameters.find(paramName) == myShaderResources.materialBuffer.parameters.end())
		{
			VT_CORE_WARN("Trying to get parameter {0} in material {1} with shader {2}. However a parameter with that name does not exist!", paramName, myName, myShader->GetName());
			return T{};
		}

		// #TODO(Ivar): Add way to make sure the type is correct
		const auto& param = myShaderResources.materialBuffer.parameters.at(paramName);
		return *(T*)&myShaderResources.materialBuffer.data[param.offset];
	}
}