#include "vtpch.h"
#include "Shader.h"

#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Core/Graphics/GraphicsContext.h"

#include "Volt/Rendering/Shader/ShaderCompiler.h"
#include "Volt/Rendering/Shader/ShaderUtility.h"
#include "Volt/Rendering/Buffer/ConstantBufferRegistry.h"
#include "Volt/Rendering/Buffer/StructuredBufferRegistry.h"
#include "Volt/Rendering/Buffer/ConstantBuffer.h"
#include "Volt/Rendering/Buffer/StructuredBuffer.h"
#include "Volt/Utility/DirectXUtils.h"

#include "Volt/Log/Log.h"

namespace Volt
{
	Shader::Shader(const std::string& aName, std::initializer_list<std::filesystem::path> aPaths, bool aForceCompile)
		: mySourcePaths(aPaths), myName(aName)
	{
		if (myBindFunctions.empty())
		{
			SetupBindAndCreateFunctions();
		}

		Reload(aForceCompile);
		GenerateHash();
	}

	Shader::Shader(const std::string& aName, std::vector<std::filesystem::path> aPaths, bool aForceCompile, bool aIsInternal)
		: mySourcePaths(aPaths), myName(aName), myIsInternal(aIsInternal)
	{
		if (myBindFunctions.empty())
		{
			SetupBindAndCreateFunctions();
		}

		Reload(aForceCompile);
		GenerateHash();
	}

	Shader::~Shader()
	{
		Release();
	}

	bool Shader::Reload(bool forceCompile)
	{
		std::unordered_map<ShaderStage, ComPtr<ID3D10Blob>> tempBlobs;

		{
			Shader::ShaderResources tempResources = myResources;
			myResources = Shader::ShaderResources{};
			myResources.shaderTextureDefinitions = tempResources.shaderTextureDefinitions;
		}

		myPerStageCBCount.clear();
		myPerStageTextureCount.clear();

		if (!CompileOrGetBinary(tempBlobs, forceCompile))
		{
			return false;
		}

		myShaderBlobs.clear();
		myShaders.clear();

		myShaderBlobs = tempBlobs;

		auto device = GraphicsContext::GetDevice();
		for (const auto& [stage, blob] : myShaderBlobs)
		{
			myCreateFunctions[stage](myShaders[stage], blob, device);
		}

		ReflectAllStages();

		for (const auto& material : myMaterialReferences)
		{
			material->Invalidate();
		}

		VT_CORE_INFO("Shader {0} loaded!", myName);

		return true;
	}

	void Shader::Bind() const
	{
		auto context = GraphicsContext::GetContext();

		if (!myShaders.contains(ShaderStage::Compute))
		{
			context->IASetInputLayout(myInputLayout.Get());
		}

		for (const auto& [stage, shader] : myShaders)
		{
			myBindFunctions[stage](shader, context);
		}
	}

	void Shader::Unbind() const
	{
		auto context = GraphicsContext::GetContext();
		for (const auto& [stage, shader] : myShaders)
		{
			myUnbindFunctions[stage](shader, context);
		}
	}

	void Shader::AddReference(SubMaterial* material)
	{
		if (auto it = std::find(myMaterialReferences.begin(), myMaterialReferences.end(), material); it != myMaterialReferences.end())
		{
			VT_CORE_ERROR("Shader {0} already has a reference to material!", myName.c_str());
			return;
		}

		std::scoped_lock lock(myReferencesMutex);
		myMaterialReferences.emplace_back(material);
	}

	void Shader::RemoveReference(SubMaterial* material)
	{
		auto it = std::find(myMaterialReferences.begin(), myMaterialReferences.end(), material);
		if (it == myMaterialReferences.end())
		{
			VT_CORE_ERROR("Reference to material not found in shader {0}!", myName.c_str());
			return;
		}

		std::scoped_lock lock(myReferencesMutex);
		myMaterialReferences.erase(it);
	}

	Ref<Shader> Shader::Create(const std::string& aName, std::initializer_list<std::filesystem::path> aPaths, bool aForceCompile)
	{
		return CreateRef<Shader>(aName, aPaths, aForceCompile);
	}

	Ref<Shader> Shader::Create(const std::string& aName, std::vector<std::filesystem::path> aPaths, bool aForceCompile, bool aIsInternal)
	{
		return CreateRef<Shader>(aName, aPaths, aForceCompile, aIsInternal);
	}

	void Shader::GenerateHash()
	{
		size_t hash = std::hash<std::string>()(myName);
		for (const auto& shaderPath : mySourcePaths)
		{
			size_t pathHash = std::filesystem::hash_value(shaderPath);
			hash = Utility::HashCombine(hash, pathHash);
		}

		myHash = hash;
	}

	void Shader::Release()
	{
		myShaderBlobs.clear();
		myShaders.clear();
	}

	void Shader::SetupBindAndCreateFunctions()
	{
		/////Bind/////
		myBindFunctions[ShaderStage::Compute] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->CSSetShader((ID3D11ComputeShader*)shader.Get(), nullptr, 0);
		};

		myBindFunctions[ShaderStage::Domain] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->DSSetShader((ID3D11DomainShader*)shader.Get(), nullptr, 0);
		};

		myBindFunctions[ShaderStage::Geometry] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->GSSetShader((ID3D11GeometryShader*)shader.Get(), nullptr, 0);
		};

		myBindFunctions[ShaderStage::Hull] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->HSSetShader((ID3D11HullShader*)shader.Get(), nullptr, 0);
		};

		myBindFunctions[ShaderStage::Pixel] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->PSSetShader((ID3D11PixelShader*)shader.Get(), nullptr, 0);
		};

		myBindFunctions[ShaderStage::Vertex] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->VSSetShader((ID3D11VertexShader*)shader.Get(), nullptr, 0);
		};

		/////Unbind/////
		myUnbindFunctions[ShaderStage::Compute] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->CSSetShader(nullptr, nullptr, 0);
		};

		myUnbindFunctions[ShaderStage::Domain] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->DSSetShader(nullptr, nullptr, 0);
		};

		myUnbindFunctions[ShaderStage::Geometry] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->GSSetShader(nullptr, nullptr, 0);
		};

		myUnbindFunctions[ShaderStage::Hull] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->HSSetShader(nullptr, nullptr, 0);
		};

		myUnbindFunctions[ShaderStage::Pixel] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->PSSetShader(nullptr, nullptr, 0);
		};

		myUnbindFunctions[ShaderStage::Vertex] = [](ComPtr<ID3D11DeviceChild> shader, ComPtr<ID3D11DeviceContext> context)
		{
			context->VSSetShader(nullptr, nullptr, 0);
		};

		/////Create/////
		myCreateFunctions[ShaderStage::Compute] = [](ComPtr<ID3D11DeviceChild>& shader, ComPtr<ID3D10Blob> blob, ComPtr<ID3D11Device> device)
		{
			VT_DX_CHECK(device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, (ID3D11ComputeShader**)shader.GetAddressOf()));
		};

		myCreateFunctions[ShaderStage::Domain] = [](ComPtr<ID3D11DeviceChild>& shader, ComPtr<ID3D10Blob> blob, ComPtr<ID3D11Device> device)
		{
			VT_DX_CHECK(device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, (ID3D11DomainShader**)shader.GetAddressOf()));
		};

		myCreateFunctions[ShaderStage::Geometry] = [](ComPtr<ID3D11DeviceChild>& shader, ComPtr<ID3D10Blob> blob, ComPtr<ID3D11Device> device)
		{
			VT_DX_CHECK(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, (ID3D11GeometryShader**)shader.GetAddressOf()));
		};

		myCreateFunctions[ShaderStage::Hull] = [](ComPtr<ID3D11DeviceChild>& shader, ComPtr<ID3D10Blob> blob, ComPtr<ID3D11Device> device)
		{
			VT_DX_CHECK(device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, (ID3D11HullShader**)shader.GetAddressOf()));
		};

		myCreateFunctions[ShaderStage::Pixel] = [](ComPtr<ID3D11DeviceChild>& shader, ComPtr<ID3D10Blob> blob, ComPtr<ID3D11Device> device)
		{
			VT_DX_CHECK(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, (ID3D11PixelShader**)shader.GetAddressOf()));
		};

		myCreateFunctions[ShaderStage::Vertex] = [](ComPtr<ID3D11DeviceChild>& shader, ComPtr<ID3D10Blob> blob, ComPtr<ID3D11Device> device)
		{
			VT_DX_CHECK(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, (ID3D11VertexShader**)shader.GetAddressOf()));
		};
	}

	void Shader::ReflectAllStages()
	{
#ifdef VT_SHADER_PRINT
		VT_CORE_INFO("Shader - Reflecting {0}", myName.c_str());
#endif

		for (const auto& [stage, blob] : myShaderBlobs)
		{
			ReflectStage(stage, blob);
		}
	}

	void Shader::ReflectStage(ShaderStage aStage, ComPtr<ID3D10Blob> aBlob)
	{
#ifdef VT_SHADER_PRINT
		VT_CORE_INFO("	Reflecting stage {0}", Utility::StageToString(aStage).c_str());
#endif

		ID3D11ShaderReflection* reflector = nullptr;
		VT_DX_CHECK(D3DReflect(aBlob->GetBufferPointer(), aBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector));

		D3D11_SHADER_DESC shaderDesc{};
		reflector->GetDesc(&shaderDesc);

		for (uint32_t i = 0; i < shaderDesc.BoundResources; i++)
		{
			D3D11_SHADER_INPUT_BIND_DESC desc{};

			reflector->GetResourceBindingDesc(i, &desc);

			switch (desc.Type)
			{
				case D3D_SIT_CBUFFER:
				{
					myResources.constantBuffers[desc.BindPoint] = { desc.Name };
					myPerStageCBCount[aStage].count++;

					if (desc.BindPoint != MATERIAL_BUFFER_BINDING)
					{
						if (auto buffer = ConstantBufferRegistry::Get(desc.BindPoint))
						{
							buffer->AddStage(aStage);
						}
					}
					else
					{
						myResources.materialBuffer.exists = true;
						myResources.materialBuffer.name = desc.Name;
					}

					break;
				}

				case D3D_SIT_STRUCTURED:
				{
					if (desc.BindPoint)
					{
						if (auto buffer = StructuredBufferRegistry::Get(desc.BindPoint))
						{
							buffer->AddStage(aStage);
						}
					}

					break;
				}

				case D3D_SIT_TEXTURE:
				{
					ImageDimension dim = ImageDimension::Dim2D;

					switch (desc.Dimension)
					{
						case D3D_SRV_DIMENSION_TEXTURE1D:
							dim = ImageDimension::Dim1D;
							break;

						case D3D_SRV_DIMENSION_TEXTURE2D:
							dim = ImageDimension::Dim2D;
							break;

						case D3D_SRV_DIMENSION_TEXTURE3D:
							dim = ImageDimension::Dim3D;
							break;

						case D3D_SRV_DIMENSION_TEXTURECUBE:
							dim = ImageDimension::DimCube;
							break;
					}

					if (desc.BindPoint <= Shader::MaxCustomTextureSlot())
					{
						myResources.textures[desc.BindPoint] = { desc.Name, dim };
						myPerStageTextureCount[aStage].count++;
					}
				}
			}
		}

		for (uint32_t i = 0; i < shaderDesc.ConstantBuffers; i++)
		{
			ID3D11ShaderReflectionConstantBuffer* buffer = reflector->GetConstantBufferByIndex(i);
			D3D11_SHADER_BUFFER_DESC desc{};

			VT_DX_CHECK(buffer->GetDesc(&desc));

			if (desc.Name == myResources.materialBuffer.name)
			{
				auto& materialBuffer = myResources.materialBuffer;
				materialBuffer.size = desc.Size;
				materialBuffer.data.resize(desc.Size);

				for (uint32_t j = 0; j < desc.Variables; j++)
				{
					ID3D11ShaderReflectionVariable* shaderParam = buffer->GetVariableByIndex(j);
					D3D11_SHADER_VARIABLE_DESC varDesc{};

					VT_DX_CHECK(shaderParam->GetDesc(&varDesc));

					auto& param = materialBuffer.parameters[varDesc.Name];
					param.offset = varDesc.StartOffset;

					ID3D11ShaderReflectionType* type = shaderParam->GetType();
					D3D11_SHADER_TYPE_DESC typeDesc{};

					VT_DX_CHECK(type->GetDesc(&typeDesc));
					param.type = Utility::GetTypeFromTypeDesc(typeDesc);

					switch (param.type)
					{
						case ElementType::Bool:
							*(bool*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(bool*)varDesc.DefaultValue : false;
							break;

						case ElementType::Float:
							*(float*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(float*)varDesc.DefaultValue : 0.f;
							break;

						case ElementType::Float2:
							*(gem::vec2*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(gem::vec2*)varDesc.DefaultValue : 1.f;
							break;

						case ElementType::Float3:
							*(gem::vec3*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(gem::vec3*)varDesc.DefaultValue : 1.f;
							break;

						case ElementType::Float4:
							*(gem::vec4*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(gem::vec4*)varDesc.DefaultValue : 1.f;
							break;

						case ElementType::Int:
							*(int32_t*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(int32_t*)varDesc.DefaultValue : 0;
							break;

						case ElementType::UInt:
							*(uint32_t*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(uint32_t*)varDesc.DefaultValue : 0;
							break;

						case ElementType::UInt2:
							*(gem::vec2ui*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(gem::vec2ui*)varDesc.DefaultValue : 1;
							break;

						case ElementType::UInt3:
							*(gem::vec3ui*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(gem::vec3ui*)varDesc.DefaultValue : 1;
							break;

						case ElementType::UInt4:
							*(gem::vec4ui*)&materialBuffer.data[param.offset] = varDesc.DefaultValue ? *(gem::vec4ui*)varDesc.DefaultValue : 1;
							break;
					}
				}
			}
		}

		if (aStage == ShaderStage::Vertex)
		{
			const auto& inputs = ReflectInputDescription(reflector);
			auto device = GraphicsContext::GetDevice();
			if (!inputs.empty())
			{
				VT_DX_CHECK(device->CreateInputLayout(inputs.data(), (uint32_t)inputs.size(), myShaderBlobs[ShaderStage::Vertex]->GetBufferPointer(), myShaderBlobs[ShaderStage::Vertex]->GetBufferSize(), myInputLayout.GetAddressOf()));
			}
		}

#ifdef VT_SHADER_PRINT
		VT_CORE_INFO("		Constant Buffers: {0}", myPerStageCBCount[aStage].count);
		VT_CORE_INFO("		Textures: {0}", myPerStageTextureCount[aStage].count);
#endif

		reflector->Release();
			}

	std::vector<D3D11_INPUT_ELEMENT_DESC> Shader::ReflectInputDescription(ID3D11ShaderReflection* aReflector)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputs;

		D3D11_SHADER_DESC shaderDesc{};
		aReflector->GetDesc(&shaderDesc);

		D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
		D3D11_INPUT_ELEMENT_DESC inputElement;

		uint32_t byteOffset = 0;
		uint32_t slot = 0;
		uint32_t lastSlot = 0;

		const std::string instanceName = "SV_InstanceID";
		const std::string indexName = "SV_VertexID";
		const std::string primitiveName = "SV_PrimitiveID";

		for (uint32_t i = 0; i < shaderDesc.InputParameters; i++)
		{
			aReflector->GetInputParameterDesc(i, &inputDesc);

			inputElement.SemanticName = inputDesc.SemanticName;

			const std::string semanticName = inputElement.SemanticName;
			if (semanticName == instanceName || semanticName == indexName || semanticName == primitiveName)
			{
				continue;
			}

			const bool isInstanceData = semanticName.contains("_INSTANCE");
			if (isInstanceData)
			{
				slot = 1;
			}

			if (lastSlot != slot)
			{
				byteOffset = 0;
				lastSlot = slot;
			}

			inputElement.SemanticIndex = inputDesc.SemanticIndex;
			inputElement.InputSlot = slot;
			inputElement.AlignedByteOffset = byteOffset;
			inputElement.InstanceDataStepRate = isInstanceData ? 1 : 0;
			inputElement.InputSlotClass = isInstanceData ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;

			switch (inputDesc.Mask)
			{
				case 1:
					if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32_UINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32_SINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32_FLOAT;
					byteOffset += 4;
					inputs.push_back(inputElement);
					break;
				case 3:
					if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32G32_UINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32G32_SINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32G32_FLOAT;
					byteOffset += 8;
					inputs.push_back(inputElement);
					break;
				case 7:
					if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32G32B32_UINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32G32B32_SINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
					byteOffset += 12;
					inputs.push_back(inputElement);
					break;
				case 15:
					if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) inputElement.Format = DXGI_FORMAT_R32G32B32A32_UINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) inputElement.Format = DXGI_FORMAT_R32G32B32A32_SINT;
					else if (inputDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) inputElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					byteOffset += 16;
					inputs.push_back(inputElement);
					break;
			}
		}

		return inputs;
	}

	bool Shader::CompileOrGetBinary(std::unordered_map<ShaderStage, ComPtr<ID3D10Blob>>& outBlobs, bool aForceCompile)
	{
		const auto cacheDirectory = Utility::GetShaderCacheDirectory();

		bool failedToRead = false;

		if (!aForceCompile)
		{
			for (const auto& shaderPath : mySourcePaths)
			{
				const auto stage = Utility::GetStageFromPath(shaderPath);
				const auto extension = Utility::GetShaderStageCachedFileExtension(stage);
				const auto cachedPath = cacheDirectory / (shaderPath.filename().string() + extension);

				std::ifstream file(cachedPath.string(), std::ios::binary | std::ios::in | std::ios::ate);
				if (file.is_open())
				{
					uint64_t size = file.tellg();

					D3DCreateBlob(size, &outBlobs[stage]);

					file.seekg(0, std::ios::beg);
					file.read((char*)outBlobs[stage]->GetBufferPointer(), size);
					file.close();
				}
				else
				{
					failedToRead = true;
					break;
				}
			}
		}

		if (failedToRead || aForceCompile || outBlobs.empty())
		{
			bool compiled = ShaderCompiler::TryCompile(mySourcePaths, outBlobs);
			if (!compiled)
			{
				return false;
			}
		}

		return true;
	}
		}