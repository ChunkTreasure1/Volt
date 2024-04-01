#pragma once

#define DECLARE_DELEGATE(DelegateName, ReturnType, ...) \
	typedef Delegate<ReturnType(__VA_ARGS__)> DelegateName;
