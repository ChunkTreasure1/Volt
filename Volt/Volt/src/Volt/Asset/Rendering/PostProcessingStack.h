#pragma once

#include "Volt/Asset/Asset.h"

namespace Volt
{
	class PostProcessingMaterial;

	struct PostProcessingEffect
	{
		AssetHandle materialHandle;
	};

	class PostProcessingStack : public Asset
	{
	public:
		PostProcessingStack() = default;
		~PostProcessingStack() override = default;

		void PushEffect(PostProcessingEffect material);
		void PopEffect();

		void InsertEffect(PostProcessingEffect material, uint32_t index);
		void RemoveEffect(uint32_t index);

		const std::vector<PostProcessingEffect>& GetAllEffects() const { return myPostProcessingStack; }
		std::vector<PostProcessingEffect>& GetAllEffectsMutable() { return myPostProcessingStack; }

		static AssetType GetStaticType() { return AssetType::PostProcessingStack; }
		AssetType GetType() override { return GetStaticType(); }

	private:
		friend class PostProcessingStackImporter;

		std::vector<PostProcessingEffect> myPostProcessingStack;
	};
}
