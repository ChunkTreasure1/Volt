#pragma once
#include "Nexus/Core/Types.h"

namespace Nexus
{
	class Replicated
	{
	public:
		Replicated() {}
		Replicated(TYPE::eReplicatedType _type, TYPE::CLIENT_ID owner) : m_type(_type), m_ownerId(owner) {}
		TYPE::eReplicatedType GetType() { return m_type; }
		TYPE::CLIENT_ID GetOwner() { return m_ownerId; }
	private:
		const TYPE::eReplicatedType m_type = TYPE::eReplicatedType::NIL;
		const TYPE::CLIENT_ID m_ownerId = 0;
	};
}
