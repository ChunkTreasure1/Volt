#pragma once

#include <CoreUtilities/Core.h>
#include <CoreUtilities/VoltAssert.h>

#include <memory>
#include <string>

#ifdef RHIMODULE_DLL_EXPORT
#define VTRHI_API __declspec(dllexport)
#else
#define VTRHI_API __declspec(dllimport)
#endif
