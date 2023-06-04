#pragma once

#include <GraphKey/Graph.h>

namespace Volt
{
	class AnimationGraphAsset : public GraphKey::Graph
	{
	public:
		inline AnimationGraphAsset() = default;
		inline AnimationGraphAsset(AssetHandle character)
			: myAnimatedCharacter(character)
		{
		}

		inline AnimationGraphAsset(AssetHandle character, Wire::EntityId entity)
			: GraphKey::Graph(entity), myAnimatedCharacter(character)
		{
		}

		~AnimationGraphAsset() override = default;

		void SetCharacterHandle(AssetHandle handle);
		inline const AssetHandle GetCharacterHandle() const { return myAnimatedCharacter; }
		inline void SetState(const std::string& state) { myGraphState = state; }
		inline const std::string& GetState() const { return myGraphState; }

		inline const float GetStartTime() const { return myStartTime; }
		inline void SetStartTime(float startTime) { myStartTime = startTime; }

		static AssetType GetStaticType() { return Volt::AssetType::AnimationGraph; }
		AssetType GetType() override { return GetStaticType(); };

		Ref<AnimationGraphAsset> CreateCopy(Wire::EntityId entity = 0);

	private:
		friend class AnimationGraphImporter;

		AssetHandle myAnimatedCharacter = Asset::Null();
		std::string myGraphState;
	
		float myStartTime = 0.f;
	};
}
