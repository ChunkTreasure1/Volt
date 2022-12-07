#include "vtpch.h"
#include "DDSTextureImporter.h"

#include "Volt/Core/Graphics/GraphicsContext.h"

#include "Volt/Rendering/Texture/ImageCommon.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Utility/DirectXUtils.h"

#include <DDSTextureLoader.h>

namespace Volt
{
	Ref<Texture2D> DDSTextureImporter::ImportTextureImpl(const std::filesystem::path& path)
	{
		auto device = GraphicsContext::GetDevice();
		Ref<Image2D> image = CreateRef<Image2D>();

		VT_DX_CHECK(DirectX::CreateDDSTextureFromFile(device.Get(), path.wstring().c_str(), (ID3D11Resource**)image->myTexture.GetAddressOf(), image->myResourceView.GetAddressOf()));

		D3D11_TEXTURE2D_DESC desc;
		image->myTexture->GetDesc(&desc);

		const uint32_t width = desc.Width;
		const uint32_t height = desc.Height;

		image->mySpecification.width = width;
		image->mySpecification.height = height;

		Ref<Texture2D> texture = CreateRef<Texture2D>();
		texture->myImage = image;

		return texture;
	}
}