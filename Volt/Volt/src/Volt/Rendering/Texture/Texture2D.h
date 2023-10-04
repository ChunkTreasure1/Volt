#pragma once

#include "Volt/Asset/Asset.h"

#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	namespace RHI
	{
		class Image2D;
	}

	class Texture2D : public Asset
	{
	public:
		Texture2D(RHI::PixelFormat format, uint32_t width, uint32_t height, const void* data);
		Texture2D(Ref<RHI::Image2D> image);
		Texture2D() = default;
		~Texture2D() override;

		const uint32_t GetWidth() const;
		const uint32_t GetHeight() const;

		inline const Ref<RHI::Image2D> GetImage() const { return m_image; }
		inline void SetImage(Ref<RHI::Image2D> image) { m_image = image; }

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Texture2D> Create(RHI::PixelFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static Ref<Texture2D> Create(Ref<RHI::Image2D> image);

	private:
		friend class DDSTextureImporter;
		friend class DefaultTextureImporter;

		Ref<RHI::Image2D> m_image;
	};
}
