#pragma once

#include <CoreUtilities/Core.h>
#include <CoreUtilities/Assert.h>

#include <memory>
#include <string>

#ifdef VTRHI_BUILD_DLL
#define VTRHI_API __declspec(dllexport)
#else
#define VTRHI_API __declspec(dllimport)
#endif
