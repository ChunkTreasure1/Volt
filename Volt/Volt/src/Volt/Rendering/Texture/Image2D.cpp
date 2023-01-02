#include "vtpch.h"
#include "Image2D.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	Image2D::Image2D(const ImageSpecification& aSpecification, const void* aData)
		: mySpecification(aSpecification)
	{
		Invalidate(aData);
	}

	Image2D::~Image2D()
	{
		Release();
	}

	void Image2D::Invalidate(const void* aData)
	{
		Release();

		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = mySpecification.width;
		texDesc.Height = mySpecification.height;
		texDesc.MipLevels = mySpecification.mips;
		texDesc.ArraySize = mySpecification.layers;

		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Format = Utility::VoltToDXFormat(mySpecification.format);

		myIsDepth = Utility::IsDepthFormat(mySpecification.format);

		const bool isTypeless = Utility::IsTypeless(mySpecification.format);

		switch (mySpecification.usage)
		{
			case ImageUsage::Attachment: texDesc.Usage = D3D11_USAGE_DEFAULT; break;
			case ImageUsage::AttachmentStorage: texDesc.Usage = D3D11_USAGE_DEFAULT; break;
			case ImageUsage::Storage: texDesc.Usage = D3D11_USAGE_DEFAULT; break;
			case ImageUsage::Texture:
			{
				if (mySpecification.readable || mySpecification.writeable)
				{
					texDesc.Usage = D3D11_USAGE_DYNAMIC;
				}
				else
				{
					texDesc.Usage = D3D11_USAGE_IMMUTABLE;
				}
			}
			break;
		}

		switch (mySpecification.usage)
		{
			case ImageUsage::Attachment:
			{
				if (myIsDepth || isTypeless)
				{
					texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
					if (isTypeless)
					{
						texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
					}
				}
				else
				{
					texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				}

				break;
			}

			case ImageUsage::AttachmentStorage:
			{
				if (myIsDepth || isTypeless)
				{
					texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
					if (isTypeless)
					{
						texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
					}
				}
				else
				{
					texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
				}

				break;
			}

			case ImageUsage::Texture:
			{
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				break;
			}

			case ImageUsage::Storage:
			{
				texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
				break;
			}
		}

		if (mySpecification.isCubeMap)
		{
			texDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		if (mySpecification.readable)
		{
			texDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		if (mySpecification.writeable)
		{
			texDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		}

		const uint32_t bytesPerPixel = Utility::PerPixelSizeFromFormat(mySpecification.format);
		const uint32_t perLayerDataOffset = mySpecification.width * bytesPerPixel;
		const uint8_t* byteData = (uint8_t*)aData;

		std::vector<D3D11_SUBRESOURCE_DATA> subData{};

		for (uint32_t i = 0; i < mySpecification.layers; i++)
		{
			auto& data = subData.emplace_back();
			data.pSysMem = &byteData[perLayerDataOffset * i];
			data.SysMemPitch = perLayerDataOffset;
		}

		auto device = GraphicsContext::GetDevice();
		VT_DX_CHECK(device->CreateTexture2D(&texDesc, aData ? subData.data() : nullptr, myTexture.GetAddressOf()));

		if (!mySpecification.debugName.empty())
		{
			myTexture->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32_t)mySpecification.debugName.size(), mySpecification.debugName.c_str());
		}

		if (mySpecification.usage != ImageUsage::Storage)
		{
			if (Utility::IsDepthFormat(mySpecification.format) || isTypeless)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
				desc.Format = Utility::VoltToDXFormat(isTypeless ? mySpecification.typelessViewFormat : mySpecification.format);

				if (mySpecification.isCubeMap)
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.ArraySize = 6;
					desc.Texture2DArray.FirstArraySlice = 0;
					desc.Texture2DArray.MipSlice = 0;
				}
				else
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					desc.Texture2D.MipSlice = 0;
				}


				device->CreateDepthStencilView(myTexture.Get(), &desc, (ID3D11DepthStencilView**)myTargetView.GetAddressOf());
			}
			else if (mySpecification.usage == ImageUsage::Attachment || mySpecification.usage == ImageUsage::AttachmentStorage)
			{
				device->CreateRenderTargetView(myTexture.Get(), nullptr, (ID3D11RenderTargetView**)myTargetView.GetAddressOf());
			}
		}

		if (!Utility::IsDepthFormat(mySpecification.format) || isTypeless)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc{};
			shaderDesc.Format = isTypeless ? Utility::VoltToDXFormat(mySpecification.typelessTargetFormat) : Utility::VoltToDXFormat(mySpecification.format);
			shaderDesc.ViewDimension = mySpecification.isCubeMap ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;

			if (shaderDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURECUBE)
			{
				shaderDesc.TextureCube.MipLevels = mySpecification.mips;
				shaderDesc.TextureCube.MostDetailedMip = 0;
			}
			else
			{
				shaderDesc.Texture2D.MipLevels = texDesc.MipLevels;
				shaderDesc.Texture2D.MostDetailedMip = 0;
			}

			VT_DX_CHECK(device->CreateShaderResourceView(myTexture.Get(), &shaderDesc, myResourceView.GetAddressOf()));
		}

		if ((mySpecification.usage == ImageUsage::Storage || mySpecification.usage == ImageUsage::AttachmentStorage) && !Utility::IsDepthFormat(mySpecification.format))
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC desc{};
			desc.Format = isTypeless ? Utility::VoltToDXFormat(mySpecification.typelessTargetFormat) : Utility::VoltToDXFormat(mySpecification.format);
			desc.ViewDimension = mySpecification.layers > 1 ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;

			if (desc.ViewDimension == D3D11_UAV_DIMENSION_TEXTURE2DARRAY)
			{
				desc.Texture2DArray.MipSlice = mySpecification.mipSlice;
				desc.Texture2DArray.FirstArraySlice = 0;
				desc.Texture2DArray.ArraySize = mySpecification.layers;
			}

			VT_DX_CHECK(device->CreateUnorderedAccessView(myTexture.Get(), &desc, myMipUAVs[0].GetAddressOf()));
		}
	}

	void Image2D::Release()
	{
		myTexture.Reset();
		myTargetView.Reset();
		myResourceView.Reset();
		myMipUAVs.clear();
	}

	const Buffer Image2D::GetDataBuffer() const
	{
		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = mySpecification.width;
		desc.Height = mySpecification.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = Utility::VoltToDXFormat(mySpecification.format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;

		HRESULT result;
		ID3D11Texture2D* texture = nullptr;
		result = GraphicsContext::GetDevice()->CreateTexture2D(&desc, nullptr, &texture);
		assert(SUCCEEDED(result));

		auto context = GraphicsContext::GetImmediateContext();
		context->CopyResource(texture, myTexture.Get());

		const uint32_t textureSize = mySpecification.width * mySpecification.height * Utility::PerPixelSizeFromFormat(mySpecification.format);

		Buffer outputBuffer{};
		outputBuffer.Allocate(textureSize);

		D3D11_MAPPED_SUBRESOURCE subresource = {};
		context->Map(texture, 0, D3D11_MAP_READ, 0, &subresource);

		outputBuffer.Copy(subresource.pData, textureSize);

		context->Unmap(texture, 0);
		texture->Release();
		texture = nullptr;

		return outputBuffer;
	}

	void Image2D::CreateMipUAVs()
	{
		for (uint32_t i = 1; i < mySpecification.mips; i++)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
			uavDesc.Format = mySpecification.format == ImageFormat::R32Typeless ? Utility::VoltToDXFormat(mySpecification.typelessTargetFormat) : Utility::VoltToDXFormat(mySpecification.format);
			uavDesc.ViewDimension = mySpecification.isCubeMap ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;

			if (mySpecification.isCubeMap)
			{
				uavDesc.Texture2DArray.MipSlice = i;
				uavDesc.Texture2DArray.FirstArraySlice = 0;
				uavDesc.Texture2DArray.ArraySize = mySpecification.layers;
			}
			else
			{
				uavDesc.Texture2D.MipSlice = i;
			}

			auto device = GraphicsContext::GetDevice();

			VT_DX_CHECK(device->CreateUnorderedAccessView(myTexture.Get(), &uavDesc, myMipUAVs[i].GetAddressOf()));
		}
	}

	void Image2D::Unmap()
	{
		auto context = GraphicsContext::GetImmediateContext();
		context->Unmap(myTexture.Get(), 0);
	}

	Ref<Image2D> Image2D::Create(const ImageSpecification& aSpecification, const void* aData)
	{
		return CreateRef<Image2D>(aSpecification, aData);
	}
}