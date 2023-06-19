#pragma once
#include <string_view>

#include "VoltRHI/Core/Core.h"

/// <summary>
/// Extension is a system inorder for Volt RHI to expand with features that you might only want to use during sertain conditions.
/// This is also to isolate Volt RHI as its own ecosystem instead of getting intergrated in to Volt entire ecosystem.  
/// This leads to more readable and scalable code, another upside to this is that Volt RHI can handle future api extensions or platform limitations.
/// </summary>

#define REGISTER_EXTENSION(extension) static std::string GetName() { return #extension; }


namespace Volt
{
	class Extension
	{
	public:
		Extension() = default;
		virtual ~Extension() = default;

		VT_DELETE_COMMON_OPERATORS(Extension);



		static Ref<Extension> Create() { return CreateRefRHI<Extension>(); }


	protected:
		std::string_view m_name;
	};

	template<class T>
	inline Ref<T> GetExtensionFromContainer(const std::vector<Ref<Extension>>& extensions)
	{
		Ref<T> requestedExtension = nullptr;
		const auto requestedExtensionName = T::GetName();
		for (auto& extension : extensions)
		{
			if (requestedExtensionName != extension->GetName())
			{
				continue;
			}

			requestedExtension = extension;
		}

		return requestedExtension;
	}
}
