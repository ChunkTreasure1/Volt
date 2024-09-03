#pragma once

#include <filesystem>

namespace Volt
{
	class NativePluginModule
	{
	public:
		NativePluginModule() = default;
		NativePluginModule(const std::filesystem::path& binaryFilepath, bool externallyLoaded) noexcept;
		NativePluginModule(NativePluginModule&& other) noexcept;
		
		~NativePluginModule();

		VT_NODISCARD VT_INLINE const std::filesystem::path& GetBinaryFilepath() const { return m_binaryFilepath; }

		void Unload();

	private:
		std::filesystem::path m_binaryFilepath;
		bool m_externallyLoaded = false;
	};
}
