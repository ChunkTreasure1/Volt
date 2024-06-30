#pragma once

#include <CoreUtilities/Buffer/Buffer.h>

#include <filesystem>

namespace Volt
{
	class DDSUtility
	{
	public:
		struct TextureData
		{
			Buffer dataBuffer{};
			uint32_t width = 0;
			uint32_t height = 0;
		};

		static const TextureData GetRawDataFromDDS(const std::filesystem::path& path);

	private:
		DDSUtility() = delete;
	};
}