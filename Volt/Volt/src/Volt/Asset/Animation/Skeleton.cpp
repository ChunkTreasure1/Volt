#include "vtpch.h"
#include "Skeleton.h"

#include "Volt/Log/Log.h"

namespace Volt
{
	const size_t Skeleton::GetJointIndexFromName(const std::string& str)
	{
		for (size_t i = 0; const auto& joint : myJoints)
		{
			if (joint.name == str)
			{
				return i;
			}
			i++;
		}

		return 0;
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