#pragma once

#include "Volt/Rendering/FrameGraph/FrameGraphResource.h"

namespace Volt
{
	class Image2D;
	class GPUImageMemoryPool;
	class TransientResourceSystemOld
	{
	public:
		TransientResourceSystemOld();
		~TransientResourceSystemOld();

		void Clear();
		void Reset();

		Weak<Image2D> AquireTexture(const FrameGraphTextureSpecification& textureSpecification, FrameGraphResourceHandle resourceHandle);

	private:
		struct ImageData
		{
			Ref<Image2D> image;
			FrameGraphTextureSpecification specification;
		};

		size_t GetHash(const FrameGraphTextureSpecification& textureSpecification);

		Ref<GPUImageMemoryPool> myImagePool;

		std::map<FrameGraphResourceHandle, ImageData> myImageResources;
		std::unordered_map<size_t, std::vector<ImageData>> myCachedImageResources;
	
		std::mutex myAquireMutex;
	};
}