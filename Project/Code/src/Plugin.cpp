#include "Plugin.h"

#include <LogModule/Log.h>

void GamePlugin::Initialize()
{
	VT_LOG(Trace, "Hello from GamePlugin!");
}

void GamePlugin::Shutdown()
{
}
