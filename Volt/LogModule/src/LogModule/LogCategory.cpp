#include "LogCategory.h"

LogCategoryBase::LogCategoryBase(std::string_view categoryName, LogVerbosity categoryVerbosity)
	: m_name(categoryName), m_verbosity(categoryVerbosity)
{
}

LogCategoryBase::~LogCategoryBase()
{
}
