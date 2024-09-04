#include "LogCategory.h"

LogCategoryBase::LogCategoryBase(std::string_view categoryName, LogVerbosity categoryVerbosity)
	: m_verbosity(categoryVerbosity), m_name(categoryName)
{
}

LogCategoryBase::~LogCategoryBase()
{
}
