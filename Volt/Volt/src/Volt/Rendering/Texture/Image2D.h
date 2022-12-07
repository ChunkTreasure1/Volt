#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/Buffer.h"

#include "Volt/Core/Graphics/GraphicsContext.h"
#include "Volt/Rendering/Texture/ImageCommon.h"
#include "Volt/Utility/ImageUtility.h"
#include "Volt/Utility/DirectXUtils.h"

#include <wrl.h>
#include <d3d11.h>
#include <cassert>
#include <vector>

using namespace Microsoft::WRL;

namespace Volt
{
	class Image2D
	{
	public:
		Image2D(const ImageSpecification& aSpecification, const void* aData);
		Image2D() = default;
		~Image2D();

		void Invalidate(const void* aData = nullptr);
		void Release();

		inline const ImageFormat GetFormat() const { return mySpecification.format; }
		inline const ImageSpecification& GetSpecification() const { return mySpecification; }

		inline const uint32_t GetWidth() const { return mySpecification.width; }
		inline const uint32_t GetHeight() const { return mySpecification.height; }
		inline const uint32_t GetMipCount() const { return mySpecification.mips; }

		inline ComPtr<ID3D11RenderTargetView> GetRTV() const { return { (ID3D11RenderTargetView*)myTargetView.Get() }; }
		inline ComPtr<ID3D11DepthStencilView> GetDSV() const { return { (ID3D11DepthStencilView*)myTargetView.Get() }; }
		inline ComPtr<ID3D11UnorderedAccessView> GetUAV(uint32_t mip = 0) const { return myMipUAVs.at(mip); }
		inline ComPtr<ID3D11ShaderResourceView> GetSRV() const { return myResourceView; }

		const Buffer GetDataBuffer() const;

		void CreateMipUAVs();

		template<typename T>
		inline T ReadPixel(uint32_t x, uint32_t y);

		template<typename T>
		inline std::vector<T> ReadPixelRange(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY);

		template<typename T>
		inline T* Map();
		void Unmap();

		static Ref<Image2D> Create(const ImageSpecification& aSpecification, const void* aData = nullptr);

	private:
		friend class DDSTextureImporter;
		friend class DefaultTextureImporter;

		ImageSpecification mySpecification;
		bool myIsDepth = false;

		ComPtr<ID3D11Texture2D> myTexture;
		ComPtr<ID3D11View> myTargetView;
		ComPtr<ID3D11ShaderResourceView> myResourceView;

		std::unordered_map<uint32_t, ComPtr<ID3D11UnorderedAccessView>> myMipUAVs;
	};

	template<typename T>
	inline std::vector<T> Image2D::ReadPixelRange(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY)
	{
		assert(minX >= 0 && minX <= mySpecification.width);
		assert(minY >= 0 && minY <= mySpecification.height);

		assert(maxX >= 0 && maxX <= mySpecification.width);
		assert(maxY >= 0 && maxY <= mySpecification.height);

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

		auto context = GraphicsContext::GetContext();
		context->CopyResource(texture, myTexture.Get());

		D3D11_MAPPED_SUBRESOURCE subresource = {};

		context->Map(texture, 0, D3D11_MAP_READ, 0, &subresource);

		T* data = reinterpret_cast<T*>(subresource.pData);

		std::vector<T> returnVal;
		for (uint32_t x = minX; x < maxX; x++)
		{
			for (uint32_t y = minY; y < maxY; y++)
			{
				uint32_t loc = (x + y * subresource.RowPitch / sizeof(T));
				returnVal.emplace_back(data[loc]);
			}
		}

		context->Unmap(texture, 0);

		texture->Release();
		texture = nullptr;
		return returnVal;
	}

	template<typename T>
	inline T Image2D::ReadPixel(uint32_t x, uint32_t y)
	{
		assert(x >= 0 && x <= mySpecification.width);
		assert(y >= 0 && y <= mySpecification.height);

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
		ID3D11Texture2D* texture;
		result = GraphicsContext::GetDevice()->CreateTexture2D(&desc, nullptr, &texture);
		assert(SUCCEEDED(result));

		auto context = GraphicsContext::GetContext();
		context->CopyResource(texture, myTexture.Get());

		D3D11_MAPPED_SUBRESOURCE subresource = {};

		context->Map(texture, 0, D3D11_MAP_READ, 0, &subresource);

		T* data = reinterpret_cast<T*>(subresource.pData);

		uint32_t loc = (x + y * subresource.RowPitch / sizeof(T));
		T value = data[loc];

		context->Unmap(texture, 0);

		texture->Release();
		texture = nullptr;
		return value;
	}

	template<typename T>
	inline T* Image2D::Map()
	{
		VT_CORE_ASSERT(mySpecification.writeable, "Image has to be created with writeable flag!");

		auto context = GraphicsContext::GetContext();
		D3D11_MAPPED_SUBRESOURCE subdata{};

		VT_DX_CHECK(context->Map(myTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subdata));
		return reinterpret_cast<T*>(subdata.pData);
	}
}