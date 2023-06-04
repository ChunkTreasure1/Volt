#include "vtpch.h"
#include "Skeleton.h"

#include "Volt/Log/Log.h"

namespace Volt
{
	const bool Skeleton::JointIsDecendantOf(int32_t jointIndex, int32_t parentIndex) const
	{
		if (jointIndex < 0 || jointIndex >= (int32_t)myJoints.size())
		{
			return false;
		}

		if (myJoints.at(jointIndex).parentIndex == parentIndex)
		{
			return true;
		}

		return JointIsDecendantOf(myJoints.at(jointIndex).parentIndex, parentIndex);
	}

	const int32_t Skeleton::GetJointIndexFromName(const std::string& str)
	{
		if (myJointNameToIndex.contains(str))
		{
			return static_cast<int32_t>(myJointNameToIndex.at(str));
		}

		for (int32_t i = 0; const auto & joint : myJoints)
		{
			if (joint.name == str)
			{
				return i;
			}
			i++;
		}

		return -1;
	}

	const std::string Skeleton::GetNameFromJointIndex(int32_t index)
	{
		if (index >= (int32_t)myJoints.size())
		{
			VT_CORE_ERROR("Index is greater than joint count!");
			return "";
		}

		return myJoints.at(index).name;
	}
}
