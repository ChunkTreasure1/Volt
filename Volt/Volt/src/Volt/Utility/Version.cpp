#include "vtpch.h"
#include "Version.h"

#include "Volt/Utility/StringUtility.h"

namespace Volt
{
	Version::Version(const std::string& strValue)
	{
		Vector<std::string> strParts = Utility::SplitStringsByCharacter(strValue, '.');
		if (strParts.size() != 3)
		{
			return;
		}

		m_major = std::stoi(strParts[0]);
		m_minor = std::stoi(strParts[1]);
		m_patch = std::stoi(strParts[2]);
	}

	Version::Version(const uint32_t major, const uint32_t minor, const uint32_t patch)
		: m_major(major), m_minor(minor), m_patch(patch)
	{
	}

	Version Version::CreateFromString(const std::string& strValue)
	{
		return Version(strValue);
	}

	Version Version::Create(const uint32_t major, const uint32_t minor, const uint32_t patch)
	{
		return Version(major, minor, patch);
	}

	const std::string Version::ToString() const
	{
		return std::to_string(m_major) + "." + std::to_string(m_minor) + "." + std::to_string(m_patch);
	}

	const bool Version::IsValid() const
	{
		return m_major != 0 || m_minor != 0 || m_patch != 0;
	}
}
