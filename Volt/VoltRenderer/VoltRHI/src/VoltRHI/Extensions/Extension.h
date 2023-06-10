#pragma once
#include <string_view>

#include "VoltRHI/Core/Core.h"

/// <summary>
/// Extension is a system inorder for Volt RHI to expand with features that you might only want to use during sertain conditions.
/// This is also to isolate Volt RHI as its own ecosystem instead of getting intergrated in to Volt entire ecosystem.  
/// This leads to more readable and scalable code, another upside to this is that Volt RHI can handle future api extensions or platform limitations.
/// </summary>

namespace Volt
{
	class Extension
	{
	public:
		Extension() = default;
		virtual ~Extension() = default;

		VT_DELETE_COMMON_OPERATORS(Extension);


		VT_INLINE VT_NODISCARD std::string_view GetExtensionName() { return m_name; }

	protected:
		std::string_view m_name;
	};
}
