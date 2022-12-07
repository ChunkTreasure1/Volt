#pragma once

#include <cr.h>

#include <filesystem>

namespace Volt
{
	class DLLHandler
	{
	public:
		DLLHandler(const std::filesystem::path& aDllPath);
		~DLLHandler();

		void Load();
		void Unload();

		void Update();

	private:
		cr_plugin myDLL;
		std::filesystem::path myPath;
	};
}