#include "vtpch.h"

#define CR_HOST
#include "DLLHandler.h"

namespace Volt
{
	DLLHandler::DLLHandler(const std::filesystem::path& aDllPath)
		: myPath(aDllPath)
	{
		Load();
	}

	DLLHandler::~DLLHandler()
	{
		Unload();
	}

	void DLLHandler::Load()
	{
		if (!cr_plugin_open(myDLL, myPath.string().c_str()))
		{
			VT_CORE_ERROR("Unable to load DLL {0}!");
		}
	}

	void DLLHandler::Unload()
	{
		cr_plugin_close(myDLL);
	}

	void DLLHandler::Update()
	{
		cr_plugin_update(myDLL);
	}
}

