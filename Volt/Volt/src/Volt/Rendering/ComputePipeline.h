#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	class Shader;
	class Image2D;
	class Texture2D;
	class Framebuffer;

	class ComputePipeline
	{
	public:
		ComputePipeline(Ref<Shader> aShader);
		~ComputePipeline();

		void Execute(uint32_t aGroupX, uint32_t aGroupY, uint32_t aGroupZ);
		void Clear();
		
		void SetImage(Ref<Image2D> aImage, uint32_t aBinding);
		void SetTexture(Ref<Texture2D> aImage, uint32_t aBinding);
		void SetTarget(Ref<Image2D> aImage, uint32_t aBinding);

		static Ref<ComputePipeline> Create(Ref<Shader> aShader);

	private:
		std::vector<Ref<Image2D>> myImagesT;
		std::vector<Ref<Texture2D>> myTexturesT;
		std::vector<Ref<Image2D>> myTargetsT;


		std::unordered_map<uint32_t, Ref<Image2D>> myImages;
		std::unordered_map<uint32_t, Ref<Texture2D>> myTextures;
		std::unordered_map<uint32_t, Ref<Image2D>> myTargets;

		Ref<Shader> myShader;
	};
}