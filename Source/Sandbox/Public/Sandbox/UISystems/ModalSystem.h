#pragma once

#include "Sandbox/Modals/Modal.h"

#include <Volt/Core/Base.h>
#include <Volt/Core/UUID.h>

#include <unordered_map>

class ModalSystem
{
public:
	static void Update();

	template<typename T> static T& AddModal(const std::string& strId);
	template<typename T> static [[nodiscard]] T& GetModal(const UUID64& modalId);

	static void RemoveModal(const UUID64& modalId);

private:
	inline static std::unordered_map<UUID64, Scope<Modal>> s_modals;
};

template<typename T>
T& ModalSystem::GetModal(const UUID64& modalId)
{
	if (!s_modals.contains(modalId))
	{
		static T empty = T{ "" };
		return empty;
	}

	return reinterpret_cast<T&>(*s_modals.at(modalId));
}

template<typename T>
inline T& ModalSystem::AddModal(const std::string& strId)
{
	UUID64 newUUID = {};
	Scope<T> newModal = CreateScope<T>(strId);

	newModal->m_id = newUUID;
	s_modals[newUUID] = std::move(newModal);

	return reinterpret_cast<T&>(*s_modals[newUUID]);
}
