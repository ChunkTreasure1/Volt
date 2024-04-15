#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Rendering/Resources/ResourceHandle.h"

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

		ResourceHandle GetResourceHandle() const;

		inline const Ref<RHI::Image2D> GetImage() const { return m_image; }
		void SetImage(Ref<RHI::Image2D> image);

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }
		uint32_t GetVersion() const override { return 1; }

		static Ref<Texture2D> Create(RHI::PixelFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static Ref<Texture2D> Create(Ref<RHI::Image2D> image);

		static const auto GetRegisteredResourceHandles() { return s_registeredResourceHandles; }

	private:
		inline static std::mutex s_testMutex;
		inline static std::vector<ResourceHandle> s_registeredResourceHandles;

		Ref<RHI::Image2D> m_image;
		ResourceHandle m_resourceHandle = Resource::Invalid;
	};
}
