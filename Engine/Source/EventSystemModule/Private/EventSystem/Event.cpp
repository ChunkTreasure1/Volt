#include "eventpch.h"

#include "Event.h"

namespace Volt
{
	std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
