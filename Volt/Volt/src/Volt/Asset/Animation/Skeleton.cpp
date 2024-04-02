#include "vtpch.h"
#include "Skeleton.h"

#include "Volt/Log/Log.h"

namespace Volt
{
	const Skeleton::JointAttachment& Skeleton::GetJointAttachmentFromName(std::string_view name) const
	{
		for (const auto& attachment : m_jointAttachments)
		{
			if (attachment.name == name)
			{
				return attachment;
			}
		}

		static Skeleton::JointAttachment nullAttachment;
		return nullAttachment;
	}
	const Skeleton::JointAttachment& Skeleton::GetJointAttachmentFromID(const UUID64& id) const
	{
		for (const auto& attachment : m_jointAttachments)
		{
			if (attachment.id == id)
			{
				return attachment;
			}
		}

		static Skeleton::JointAttachment nullAttachment;
		return nullAttachment;
	}

	bool Skeleton::HasJointAttachment(std::string_view name) const
	{
		for (const auto& attachment : m_jointAttachments)
		{
			if (attachment.name == name)
			{
				return true;
			}
		}
		return false;
	}

	const bool Skeleton::JointIsDecendantOf(int32_t jointIndex, int32_t parentIndex) const
	{
		if (jointIndex < 0 || jointIndex >= (int32_t)m_joints.size())
		{
			return false;
		}

		if (m_joints.at(jointIndex).parentIndex == parentIndex)
		{
			return true;
		}

		return JointIsDecendantOf(m_joints.at(jointIndex).parentIndex, parentIndex);
	}

	const int32_t Skeleton::GetJointIndexFromName(const std::string& str)
	{
		if (m_jointNameToIndex.contains(str))
		{
			return static_cast<int32_t>(m_jointNameToIndex.at(str));
		}

		for (int32_t i = 0; const auto & joint : m_joints)
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
		if (index >= (int32_t)m_joints.size())
		{
			VT_CORE_ERROR("Index is greater than joint count!");
			return "";
		}

		return m_joints.at(index).name;
	}
}
