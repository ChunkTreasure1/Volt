#pragma once

#include "ankerl/unordered_dense.h"

namespace vt
{
	template<typename Key, typename Value>
	using map = ankerl::unordered_dense::map<Key, Value>;
}
