#pragma once


#include <map>
#include <unordered_map>
#include <set>
#include <memory>

#include <string>

#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>

#include <functional>
#include <algorithm>
#include <filesystem>

#include <glm/glm.hpp>

#include <yaml-cpp/yaml.h>

#include <LogModule/Log.h>

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>

#define DIRECTINPUT_VERSION 0x0800
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#include <windows.h>
#include <wchar.h>


#ifdef AK_WIN
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

// Windows Header Files:
#include <windows.h>
#endif
