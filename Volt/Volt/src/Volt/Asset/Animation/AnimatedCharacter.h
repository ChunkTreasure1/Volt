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
		struct Event
		{
			uint32_t frame;
			std::string name;
		};
		
		struct JointAttachment
		{
			std::string name;
			int32_t jointIndex = -1;
			UUID id{};

			gem::vec3 positionOffset = 0.f;
			gem::quat rotationOffset = { 1.f, 0.f, 0.f, 0.f };
		};

		AnimatedCharacter() = default;
		~AnimatedCharacter() override = default;

		const std::vector<gem::mat4> SampleAnimation(uint32_t index, float aStartTime, bool looping = true) const;
		const std::vector<gem::mat4> SampleAnimation(uint32_t index, uint32_t frameIndex) const;
		const float GetAnimationDuration(uint32_t index) const;

		inline const std::map<uint32_t, Ref<Animation>>& GetAnimations() const { return myAnimations; }
		inline const size_t GetAnimationCount() const { return myAnimations.size(); }
		inline const bool HasAnimationEvents(const uint32_t animationIndex) const { return myAnimationEvents.contains(animationIndex); }
		inline const std::vector<Event>& GetAnimationEvents(uint32_t animationIndex) const { return myAnimationEvents.at(animationIndex); }
		inline const std::map<uint32_t,std::vector<Event>>& GetAnimationEventsAndIndex(uint32_t animationIndex) const { return myAnimationEvents; }

		inline const std::vector<JointAttachment>& GetJointAttachments() const { return myJointAttachments; };
		const JointAttachment GetJointAttachmentFromName(const std::string& name) const;
		const JointAttachment GetJointAttachmentFromID(const UUID& id) const;
		const bool HasJointAttachment(const std::string& attachmentName) const;

		inline void SetSkeleton(Ref<Skeleton> skeleton) { mySkeleton = skeleton; }
		inline void SetSkin(Ref<Mesh> skin) { mySkin = skin; }
		inline void SetAnimation(uint32_t index, Ref<Animation> anim) { myAnimations[index] = anim; }

		void RemoveAnimation(uint32_t index);
		void RemoveAnimationEvent(const std::string& name, uint32_t frame, uint32_t animationIndex);
		void AddAnimationEvent(const std::string& name, uint32_t frame, uint32_t animationIndex);

		const int32_t GetAnimationIndexFromHandle(Volt::AssetHandle handle);

		inline Ref<Mesh> GetSkin() const { return mySkin; }
		inline Ref<Skeleton> GetSkeleton() const { return mySkeleton; }

		static AssetType GetStaticType() { return AssetType::AnimatedCharacter; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		friend class AnimatedCharacterImporter;

		Ref<Skeleton> mySkeleton;
		Ref<Mesh> mySkin;

		std::map<uint32_t, Ref<Animation>> myAnimations;
		std::map<uint32_t, std::vector<Event>> myAnimationEvents;
	
		std::vector<JointAttachment> myJointAttachments;
	};
}
