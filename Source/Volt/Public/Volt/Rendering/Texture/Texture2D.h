#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>

#include <RHIModule/Descriptors/ResourceHandle.h>
#include <RHIModule/Core/RHICommon.h>
#include <RHIModule/Images/Image.h>

namespace Volt
{
	namespace RHI
	{
		class Image;
	}

	class Texture2D : public Asset
	{
	public:
		Texture2D(RHI::PixelFormat format, uint32_t width, uint32_t height, const void* data);
		Texture2D(RefPtr<RHI::Image> image);
		Texture2D() = default;
		~Texture2D() override;

		const uint32_t GetWidth() const;
		const uint32_t GetHeight() const;

		ResourceHandle GetResourceHandle() const;

		inline const RefPtr<RHI::Image> GetImage() const { return m_image; }
		void SetImage(RefPtr<RHI::Image> image);

		static AssetType GetStaticType() { return AssetTypes::Texture; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

		static Ref<Texture2D> Create(RHI::PixelFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static Ref<Texture2D> Create(RefPtr<RHI::Image> image);

	private:
		RefPtr<RHI::Image> m_image;
		ResourceHandle m_resourceHandle = Resource::Invalid;
	};
}
