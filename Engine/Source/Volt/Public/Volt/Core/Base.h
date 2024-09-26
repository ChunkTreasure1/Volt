#pragma once

#include <CoreUtilities/Core.h>
#include <CoreUtilities/VoltAssert.h>
#include <CoreUtilities/Pointers/RefPtr.h>
#include <CoreUtilities/Pointers/WeakPtr.h>

#include <memory>
#include <string>
#include <iostream>

#define VT_VERSION Version::Create(0, 1, 5)

#define TO_NORMALIZEDRGB(r, g, b) glm::vec4{ r / 255.f, g / 255.f, b / 255.f, 1.f }
#define TO_NORMALIZEDRGBA(r, g, b, a) glm::vec4{ r / 255.f, g / 255.f, b / 255.f, a / 255.f }
