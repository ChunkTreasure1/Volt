#pragma once
#include <Nexus/Interface/Replication/Replicated.h>
#include "Volt/Scene/Entity.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Net/SceneInteraction/CallMonoMethod.h"

//#include <mono/jit/jit.h>

//#include <mono/jit/jit.h>
//#include <mono/metadata/assembly.h>
//#include <mono/metadata/class.h>
//#include <mono/metadata/object.h>

namespace Volt
{
	class RepVariable : public Nexus::Replicated
	{
	public:
		RepVariable(const std::string& in_variableName, Nexus::TYPE::REP_ID in_repOwner, Nexus::TYPE::CLIENT_ID in_ownerId, Ref<MonoScriptInstance> in_scriptInstance, MonoScriptField in_field, Wire::EntityId in_entityId)
			: Replicated(Nexus::TYPE::eReplicatedType::VARIABLE, in_ownerId)
			, m_varName(in_variableName)
			, m_repOwner(in_repOwner)
			, m_scriptInstance(in_scriptInstance)
			, m_field(in_field)
			, m_entityId(in_entityId)
		{
			//auto monoClass = m_scriptInstance.lock()->GetClass();
			//MonoScriptEngine::NetFieldSetup(monoClass.get(), m_field);
		}

		void ResetChanged() { m_changed = false; }
		bool SetValue(void* data);

		template<typename T>
		bool GetValue(T& data);

		const MonoScriptField& GetField() const { return m_field; }

	private:
		Weak<MonoScriptInstance> GetScriptInstance() { return m_scriptInstance; }

		const Weak<MonoScriptInstance> m_scriptInstance;
		MonoScriptField m_field;
		const std::string m_varName;
		const Nexus::TYPE::REP_ID m_repOwner;
		const Wire::EntityId m_entityId;
		bool m_changed = false;
	};

	inline bool RepVariable::SetValue(void* data)
	{
		// #nexus_todo: need to failsafe data type with typeid
		if (m_scriptInstance.expired()) return false;
		m_scriptInstance.lock()->SetField(m_varName, data);
		m_changed = true;

		// call bound method
		if (m_field.netData.boundFunction == "") return true;
		auto monoClass = m_scriptInstance.lock()->GetClass();
		std::string boundFunction = (monoClass->GetNamespace() + "." + monoClass->GetClassName() + "." + m_field.netData.boundFunction);
		CallMonoMethod(Entity(m_entityId, SceneManager::GetActiveScene().lock().get()), boundFunction, std::vector<uint8_t>());

		return true;
	}

	template<typename T>
	inline bool RepVariable::GetValue(T& data)
	{
		if (m_scriptInstance.expired()) return false;
		data = m_scriptInstance.lock()->GetField<T>(m_varName);
		return true;
	}
}
