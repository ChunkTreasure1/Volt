#pragma once

#include "SubSystem/Config.h"
#include "SubSystem/SubSystemInitializationStage.h"

#include <CoreUtilities/CompilerTraits.h>
#include <CoreUtilities/VoltGUID.h>

#include <CoreUtilities/Containers/Map.h>

class SubSystem;
struct RegisteredSubSystem
{
	std::function<Ref<SubSystem>()> factoryFunction;
	int32_t initializationOrder;
	SubSystemInitializationStage initializationStage;
};

class SUBSYSTEMMODULE_API SubSystemRegistry
{
public:
	SubSystemRegistry();
	~SubSystemRegistry();

	SubSystemRegistry(const SubSystemRegistry&) = delete;
	SubSystemRegistry& operator=(const SubSystemRegistry&) = delete;

	template<typename T>
	bool RegisterSubSystem(SubSystemInitializationStage initializationStage, int32_t initializationOrder)
	{
		const VoltGUID guid = T::GetStaticSubSystemGUID();

		if (m_registeredSubSystems.contains(guid))
		{
			return false;
		}

		RegisteredSubSystem& registeredSubSystem = m_registeredSubSystems[guid];
		registeredSubSystem.initializationOrder = initializationOrder;
		registeredSubSystem.initializationStage = initializationStage;
		registeredSubSystem.factoryFunction = []() 
		{
			return CreateRef<T>();
		};

		return true;
	}

	VT_INLINE const vt::map<VoltGUID, RegisteredSubSystem>& GetRegisteredSubSystems() const { return m_registeredSubSystems; }

private:
	vt::map<VoltGUID, RegisteredSubSystem> m_registeredSubSystems;
};

extern SUBSYSTEMMODULE_API SubSystemRegistry g_subSystemRegistry;

VT_INLINE SubSystemRegistry& GetSubSystemRegistry()
{
	return g_subSystemRegistry;
}

#define VT_REGISTER_SUBSYSTEM(klass, initializationStage, initializationOrder) \
	inline static bool SubSystemRegistry_ ## klass ## _Registered = GetSubSystemRegistry().RegisterSubSystem<klass>(SubSystemInitializationStage::initializationStage, initializationOrder)

#define VT_DECLARE_SUBSYSTEM(guid) \
	VT_NODISCARD VT_INLINE static constexpr VoltGUID GetStaticSubSystemGUID() \
	{ \
		return guid; \
	}
