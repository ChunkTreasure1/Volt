#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Animation/Animation.h"

#include <glm/glm.hpp>

namespace Volt
{
	class Skeleton : public Asset
	{
	public:
		struct Joint
		{
			std::string name;
			int32_t parentIndex = -1;
		};

		Skeleton() = default;
		~Skeleton() override
		{
			myJoints.clear();
			myInverseBindPose.clear();
		}

		inline const size_t GetJointCount() const { return myJoints.size(); }
		inline const std::vector<glm::mat4>& GetInverseBindPose() const { return myInverseBindPose; }
		inline const std::vector<Animation::TRS>& GetRestPose() const { return myRestPose; }
		inline const std::vector<Joint>& GetJoints() const { return myJoints; }

		const bool JointIsDecendantOf(int32_t jointIndex, int32_t parentIndex) const;

		const int32_t GetJointIndexFromName(const std::string& str);
		const std::string GetNameFromJointIndex(int32_t index);

		static AssetType GetStaticType() { return AssetType::Skeleton; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		friend class FbxImporter;
		friend class SkeletonImporter;

		std::vector<Joint> myJoints;
		std::vector<Animation::TRS> myRestPose;
		std::vector<glm::mat4> myInverseBindPose;

		std::unordered_map<std::string, size_t> myJointNameToIndex;

		std::string myName = "Skeleton";
	};
}
