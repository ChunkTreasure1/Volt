#pragma once

namespace Volt
{
	class MonoScriptGlue
	{
	public:
		static void RegisterFunctions();

	private:
		MonoScriptGlue() = delete;
	};
}