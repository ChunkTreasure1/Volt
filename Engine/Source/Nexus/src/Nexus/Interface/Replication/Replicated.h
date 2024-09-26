#pragma once
#include "Nexus/Utility/Types.h"

namespace Nexus
{
	class Replicated
	{
	public:
		Replicated() {}
		Replicated(TYPE::eReplicatedType in_type, TYPE::CLIENT_ID owner) : m_type(in_type), m_ownerId(owner) {}
		const TYPE::eReplicatedType GetType() const { return m_type; }
		const TYPE::CLIENT_ID GetOwner() const { return m_ownerId; }
	private:
		const TYPE::eReplicatedType m_type = TYPE::eReplicatedType::NIL;
		const TYPE::CLIENT_ID m_ownerId = 0;
	};
}
