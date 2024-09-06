#include "Plugin.h"

#include <LogModule/Log.h>

void ExamplePlugin::Initialize()
{
	VT_LOG(Trace, "Hello from ExamplePlugin!");
}

void ExamplePlugin::Shutdown()
{
}
