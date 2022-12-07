#pragma once

#include "Volt/Asset/Asset.h"

#include <gem/gem.h>

#include <map>

namespace Volt
{
	class Skeleton;
	class Mesh;
	class Animation;

	class AnimatedCharacter : public Asset
	{
	public:
		AnimatedCharacter() = default;
		~AnimatedCharacter() override = default;

		const std::vector<gem::mat4> SampleAnimation(uint32_t index, float aStartTime, bool looping = true) const;
		const std::vector<gem::mat4> SampleCrossfadingAnimation(uint32_t crossfadeFrom, uint32_t crossfadeTo, float fromStartTime, float toStartTime, float crossfadeSpeed = 1.f);
		const float GetCrossfadeTimeFromAnimations(uint32_t crossfadeFrom, uint32_t crossfadeTo, float crossfadeSpeed = 1.f);

		const float GetAnimationDuration(uint32_t index) const;
		inline const std::map<uint32_t, Ref<Animation>>& GetAnimations() const { return myAnimations; }
		inline const size_t GetAnimationCount() const { return myAnimations.size(); }

		inline void SetSkeleton(Ref<Skeleton> skeleton) { mySkeleton = skeleton; }
		inline void SetSkin(Ref<Mesh> skin) { mySkin = skin; }
		inline void SetAnimation(uint32_t index, Ref<Animation> anim) { myAnimations[index] = anim; }
		void RemoveAnimation(uint32_t index);

		inline Ref<Mesh> GetSkin() const { return mySkin; }
		inline Ref<Skeleton> GetSkeleton() const { return mySkeleton; }

		static AssetType GetStaticType() { return AssetType::AnimatedCharacter; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		friend class AnimatedCharacterImporter;

		Ref<Skeleton> mySkeleton;
		Ref<Mesh> mySkin;
	
		std::map<uint32_t, Ref<Animation>> myAnimations;
	};
}