#pragma once
#include "Nexus/Core/Types/Types.h"

namespace Nexus
{
	class Replicated
	{
	public:
		Replicated() {}
		Replicated(TYPE::eReplicatedType in_type, TYPE::CLIENT_ID owner) : m_type(in_type), m_ownerId(owner) {}
		TYPE::eReplicatedType GetType() { return m_type; }
		TYPE::CLIENT_ID GetOwner() { return m_ownerId; }
	private:
		const TYPE::eReplicatedType m_type = TYPE::eReplicatedType::NIL;
		const TYPE::CLIENT_ID m_ownerId = 0;
	};
}
