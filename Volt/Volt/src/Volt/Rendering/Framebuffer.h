#pragma once

#include "Volt/Rendering/Texture/Image2D.h"

#include <GEM/gem.h>
#include <vector>
#include <map>

#include <d3d11.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace Volt
{
	struct FramebufferAttachment
	{
		FramebufferAttachment()
		{ }

		FramebufferAttachment(ImageFormat aFormat, const gem::vec4 aClearColor = { 1.f, 1.f, 1.f, 1.f }, TextureBlend aBlending = TextureBlend::Alpha, const std::string& aDebugName = "", bool aStorageCompatible = false)
			: format(aFormat), clearColor(aClearColor), debugName(aDebugName), storageCompatible(aStorageCompatible), blending(aBlending)
		{ }

		ImageFormat format = ImageFormat::RGBA;
		TextureBlend blending = TextureBlend::Alpha;
		gem::vec4 clearColor = { 1.f, 1.f, 1.f, 1.f };
		std::string debugName;
		bool readable = false;
		bool writeable = false;
		bool storageCompatible = false;
		bool isCubeMap = false;
	};

	struct FramebufferSpecification
	{
		uint32_t width = 1280;
		uint32_t height = 720;

		std::vector<FramebufferAttachment> attachments;
		std::map<uint32_t, Ref<Image2D>> existingImages;
		Ref<Image2D> existingDepth;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& aSpecification);
		~Framebuffer();

		void Invalidate();
		void Bind();
		void Unbind();
		void Clear();

		void Resize(uint32_t width, uint32_t height);
		void SetColorAttachment(Ref<Image2D> image, uint32_t index);

		inline const Ref<Image2D> GetDepthAttachment() const { return myDepthAttachmentImage; }
		inline const Ref<Image2D> GetColorAttachment(uint32_t index) const { return myColorAttachmentImages[index]; }
		inline const FramebufferSpecification& GetSpecification() const { return mySpecification; }

		inline const uint32_t GetWidth() const { return myWidth; }
		inline const uint32_t GetHeight() const { return myHeight; }

		static Ref<Framebuffer> Create(const FramebufferSpecification& aSpecification);

		template<typename T>
		inline T ReadPixel(uint32_t attachment, uint32_t x, uint32_t y);

		template<typename T>
		inline std::vector<T> ReadPixelRange(uint32_t attachment, uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY);

	private:
		void Release();

		D3D11_VIEWPORT myViewport;
		FramebufferSpecification mySpecification;

		uint32_t myWidth;
		uint32_t myHeight;

		Ref<Image2D> myDepthAttachmentImage;
		std::vector<Ref<Image2D>> myColorAttachmentImages;

		ComPtr<ID3D11BlendState> myBlendState;

		bool myOwnsDepth = true;
	};

	template<typename T>
	inline std::vector<T> Framebuffer::ReadPixelRange(uint32_t attachment, uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY)
	{
		return myColorAttachmentImages[attachment]->ReadPixelRange<T>(minX, minY, maxX, maxY);
	}

	template<typename T>
	inline T Framebuffer::ReadPixel(uint32_t attachment, uint32_t x, uint32_t y)
	{
		return myColorAttachmentImages[attachment]->ReadPixel<T>(x, y);
	}
}