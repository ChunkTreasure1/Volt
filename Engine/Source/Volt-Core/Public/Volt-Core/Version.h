#pragma once

#include "Volt-Core/Config.h"

#include <string>

namespace Volt
{
	class VTCORE_API Version
	{
	public:
		Version() = default;
		Version(const std::string& strValue);
		Version(const uint32_t major, const uint32_t minor, const uint32_t patch);

		static Version CreateFromString(const std::string& strValue);
		static Version Create(const uint32_t major, const uint32_t minor, const uint32_t patch);

		[[nodiscard]] const std::string ToString() const;
		[[nodiscard]] const bool IsValid() const;

		[[nodiscard]] inline const uint32_t GetMajor() const { return m_major; }
		[[nodiscard]] inline const uint32_t GetMinor() const { return m_minor; }
		[[nodiscard]] inline const uint32_t GetPatch() const { return m_patch; }

		friend inline bool operator==(const Version& lhs, const Version& rhs)
		{
			return lhs.m_major == rhs.m_major && lhs.m_minor == rhs.m_minor && lhs.m_patch == rhs.m_patch;
		}

		friend inline bool operator!=(const Version& lhs, const Version& rhs)
		{
			return !(lhs == rhs);
		}

		friend inline bool operator<(const Version& lhs, const Version& rhs)
		{
			return lhs.m_major < rhs.m_major || (lhs.m_major == rhs.m_major && lhs.m_minor < rhs.m_major) || (lhs.m_major == rhs.m_major && lhs.m_minor == rhs.m_minor && lhs.m_patch < rhs.m_patch);
		}

		friend inline bool operator> (const Version& lhs, const Version& rhs) { return rhs < lhs; }
		friend inline bool operator<=(const Version& lhs, const Version& rhs) { return !(lhs > rhs); }
		friend inline bool operator>=(const Version& lhs, const Version& rhs) { return !(lhs < rhs); }

	private:
		uint32_t m_major = 0;
		uint32_t m_minor = 0;
		uint32_t m_patch = 0;
	};
}
