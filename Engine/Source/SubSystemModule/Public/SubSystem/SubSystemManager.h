#pragma once

#include "SubSystem/Config.h"
#include "SubSystem/SubSystemInitializationStage.h"

#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/VoltGUID.h>

class SubSystem;
class SUBSYSTEMMODULE_API SubSystemManager
{
public:
	SubSystemManager();
	~SubSystemManager();

	SubSystemManager(const SubSystemManager&) = delete;
	SubSystemManager& operator=(const SubSystemManager&) = delete;

	void InitializeSubSystems(SubSystemInitializationStage initializationStage);
	void ShutdownSubSystems(SubSystemInitializationStage initializationStage);

	template<typename T>
	static T* GetSubSystem()
	{
		constexpr VoltGUID guid = T::GetStaticSubSystemGUID();
		VT_ENSURE(s_instance->m_subSystemsMap.contains(guid));

		return reinterpret_cast<T*>(s_instance->m_subSystemsMap.at(guid).get());
	}

private:
	inline static SubSystemManager* s_instance = nullptr;

	struct SubSystemAccelerationStructure
	{
		SubSystemInitializationStage stage;
		size_t index;
	};

	vt::map <VoltGUID, Ref<SubSystem>> m_subSystemsMap;
	vt::map<SubSystemInitializationStage, Vector<Ref<SubSystem>>> m_subSystems;
};
