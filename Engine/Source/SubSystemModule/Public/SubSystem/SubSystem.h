#pragma once

#include "SubSystem/SubSystemRegistry.h"

class SubSystem
{
public:
	SubSystem() = default;
	virtual ~SubSystem() = default;

	virtual void Initialize() {}
	virtual void Shutdown() {}
};
