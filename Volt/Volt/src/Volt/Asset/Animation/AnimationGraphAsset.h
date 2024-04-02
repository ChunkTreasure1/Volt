#pragma once

#include <GraphKey/Graph.h>

namespace GraphKey
{
	struct Node;
}

namespace Volt
{
	class AnimationGraphAsset : public GraphKey::Graph
	{
	public:
		inline AnimationGraphAsset() = default;
		inline AnimationGraphAsset(AssetHandle aSkeleton)
			: mySkeletonHandle(aSkeleton)
		{
		}
		
		inline AnimationGraphAsset(AssetHandle aSkeleton, EntityID entity)
			: GraphKey::Graph(entity), mySkeletonHandle(aSkeleton)
		{
		}

		~AnimationGraphAsset() override = default;

		void SetSkeletonHandle(AssetHandle aSkeletonHandle);
		inline const AssetHandle GetSkeletonHandle() const { return mySkeletonHandle; }

		inline void SetState(const std::string& state) { myGraphState = state; }
		inline const std::string& GetState() const { return myGraphState; }

		inline const float GetStartTime() const { return myStartTime; }
		inline void SetStartTime(float startTime) { myStartTime = startTime; }

		static AssetType GetStaticType() { return Volt::AssetType::AnimationGraph; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }
		
		Ref<AnimationGraphAsset> CreateCopy(EntityID entity = Entity::NullID());

		Ref<GraphKey::Node> GetRelevantAnimationNode();
	private:
		friend class AnimationGraphImporter;
		friend class AnimationGraphSerializer;

		AssetHandle mySkeletonHandle = Asset::Null();
		std::string myGraphState;
	
		float myStartTime = 0.f;
	};
}
