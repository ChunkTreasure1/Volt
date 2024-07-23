#pragma once

#include "Config.h"
#include "CompilerTraits.h"

#include <string_view>

VTCOREUTIL_API void AssertionFailure(const char* expression);
VTCOREUTIL_API void AssertionFailure(std::string_view expression);

#ifdef VT_ENABLE_ASSERTS

#define VT_ASSERT(expression) \
	do { \
		VT_ANALASYS_ASSUME(expression); \
		(void)((expression) || (AssertionFailure(#expression), 0)); \
	} while(0)

#define VT_ASSERT_MSG(expression, message) \
	do { \
		VT_ANALASYS_ASSUME(expression); \
		(void)((expression) || (AssertionFailure(message), 0)); \
	} while(0)
#else
#define VT_ASSERT(expression)
#define VT_ASSERT_MSG(expression, message)
#endif

#ifdef VT_ENABLE_ENSURES
#define VT_ENSURE(expression) \
	do { \
		VT_ANALASYS_ASSUME(expression); \
		if (!(expression)) { AssertionFailure(#expression); } \
	} while(0)

#define VT_ENSURE_MSG(expression, message) \
	do { \
		VT_ANALASYS_ASSUME(expression); \
		if (!(expression)) { AssertionFailure(message); } \
	} while(0)

#else
#define VT_ENUSRE(expression)
#endif
