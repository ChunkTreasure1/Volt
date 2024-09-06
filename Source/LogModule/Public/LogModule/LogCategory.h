#pragma once

#include "LogModule/Config.h"
#include "LogModule/LogCommon.h"

#include <CoreUtilities/CompilerTraits.h>

#include <string_view>

class VTLOG_API LogCategoryBase
{
public:
	LogCategoryBase(std::string_view categoryName, LogVerbosity categoryVerbosity);
	~LogCategoryBase();

	VT_INLINE constexpr std::string_view GetName() const { return m_name; }
	VT_INLINE constexpr LogVerbosity GetVerbosity() const { return m_verbosity; }

private:
	LogVerbosity m_verbosity;
	const std::string_view m_name;
};

template<LogVerbosity verbosity>
class VTLOG_API LogCategory : public LogCategoryBase
{
public:
	VT_INLINE LogCategory(std::string_view categoryName)
		: LogCategoryBase(categoryName, verbosity)
	{ }
};

#define VT_DECLARE_LOG_CATEGORY(categoryName, verbosity) \
	extern class LogCategory##categoryName : public LogCategory<verbosity> \
	{ \
	public: \
		VT_INLINE LogCategory##categoryName() : LogCategory(#categoryName) {} \
	} categoryName

#define VT_DECLARE_LOG_CATEGORY_EXPORT(exportKeyword, categoryName, verbosity) \
	extern class exportKeyword LogCategory##categoryName : public LogCategory<verbosity> \
	{ \
	public: \
		VT_INLINE LogCategory##categoryName() : LogCategory(#categoryName) {} \
	} exportKeyword categoryName

#define VT_DEFINE_LOG_CATEGORY(categoryName) LogCategory##categoryName categoryName
