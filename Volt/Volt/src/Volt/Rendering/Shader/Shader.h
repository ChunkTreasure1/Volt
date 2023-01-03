#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Asset/Asset.h"
#include "Volt/Rendering/Shader/ShaderCommon.h"
#include "Volt/Rendering/Texture/ImageCommon.h"

#include "Volt/Rendering/Buffer/BufferLayout.h"

#include <wrl.h>

#include <string>
#include <filesystem>
#include <unordered_map>
#include <thread>

using namespace Microsoft::WRL;

struct ID3D11DeviceChild;
struct ID3D11ShaderReflection;
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11Device;
struct ID3D10Blob;
struct D3D11_INPUT_ELEMENT_DESC;

#define MATERIAL_BUFFER_BINDING 2

namespace Volt
{
	// Shader info:
	// Custom textures: Range 0 - 10
	// Material buffer: slot 2

	class SubMaterial;
	class Shader : public Asset
	{
	public:
		struct MaterialBuffer
		{
			struct Parameter
			{
				ElementType type;
				uint32_t offset;
			};

			std::unordered_map<std::string, Parameter> parameters;
			std::vector<uint8_t> data;

			uint32_t size = 0;
			std::string name;
			bool exists = false;
		};

		struct ConstantBuffer
		{
			std::string name;
		};

		struct Texture
		{
			std::string name;
			ImageDimension dimension;
		};

		struct ShaderResources
		{
			std::map<uint32_t, ConstantBuffer> constantBuffers; // binding -> buffer
			std::map<uint32_t, Texture> textures; // binding -> textures

			std::unordered_map<uint32_t, std::string> shaderTextureDefinitions; // binding -> name
			MaterialBuffer materialBuffer{};
		};

		Shader(const std::string& aName, std::initializer_list<std::filesystem::path> aPaths, bool aForceCompile);
		Shader(const std::string& aName, std::vector<std::filesystem::path> aPaths, bool aForceCompile, bool aIsInternal);
		Shader() = default;
		~Shader();

		bool Reload(bool forceCompile);
		void Bind() const;
		void Unbind() const;

		void AddReference(SubMaterial* material);
		void RemoveReference(SubMaterial* material);

		inline const ShaderResources& GetResources() const { return myResources; }
		inline const std::string& GetName() const { return myName; }
		inline const size_t GetHash() const { return myHash; }
		inline const bool IsInternal() const { return myIsInternal; }
		inline const std::vector<std::filesystem::path>& GetSources() const { return mySourcePaths; }

		static AssetType GetStaticType() { return AssetType::Shader; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Shader> Create(const std::string& name, std::initializer_list<std::filesystem::path> paths, bool forceCompile = false);
		static Ref<Shader> Create(const std::string& name, std::vector<std::filesystem::path> paths, bool forceCompile = false, bool aIsInternal = false);

		static const uint32_t MinCustomTextureSlot() { return 0; }
		static const uint32_t MaxCustomTextureSlot() { return 10; }

	private:
		struct TypeCount
		{
			uint32_t count = 0;
		};

		void GenerateHash();
		void Release();
		void SetupBindAndCreateFunctions();

		void ReflectAllStages();
		void ReflectStage(ShaderStage aStage, ComPtr<ID3D10Blob> aBlob);

		std::vector<D3D11_INPUT_ELEMENT_DESC> ReflectInputDescription(ID3D11ShaderReflection* aReflector);

		bool CompileOrGetBinary(std::unordered_map<ShaderStage, ComPtr<ID3D10Blob>>& outBlobs, bool aForceCompile);

		std::vector<SubMaterial*> myMaterialReferences;
		std::mutex myReferencesMutex;

		std::unordered_map<ShaderStage, ComPtr<ID3D11DeviceChild>> myShaders;
		std::unordered_map<ShaderStage, ComPtr<ID3D10Blob>> myShaderBlobs;

		ComPtr<ID3D11InputLayout> myInputLayout;
		std::vector<std::filesystem::path> mySourcePaths;
		
		size_t myHash = 0;
		std::string myName;
		bool myIsInternal = false;

		ShaderResources myResources;

		std::unordered_map<ShaderStage, TypeCount> myPerStageCBCount;
		std::unordered_map<ShaderStage, TypeCount> myPerStageTextureCount;

		inline static std::unordered_map<ShaderStage, std::function<void(ComPtr<ID3D11DeviceChild>, ComPtr<ID3D11DeviceContext>)>> myBindFunctions;
		inline static std::unordered_map<ShaderStage, std::function<void(ComPtr<ID3D11DeviceChild>, ComPtr<ID3D11DeviceContext>)>> myUnbindFunctions;
		inline static std::unordered_map<ShaderStage, std::function<void(ComPtr<ID3D11DeviceChild>&, ComPtr<ID3D10Blob>, ComPtr<ID3D11Device>)>> myCreateFunctions;
	};
}