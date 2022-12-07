#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Rendering/RenderPass.h"
#include <vector>

namespace Volt
{
	class RenderPipeline : public Asset
	{
	public:
		const uint32_t AddRenderPass();
		inline RenderPass& GetRenderPass(const uint32_t index) { return myRenderPasses.at(index); }

		static AssetType GetStaticType() { return AssetType::RenderPipeline; }
		virtual AssetType GetType() override { return GetStaticType(); }

	private:
		std::vector<RenderPass> myRenderPasses;
	};
}