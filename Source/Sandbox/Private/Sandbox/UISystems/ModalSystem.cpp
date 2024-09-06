#include "sbpch.h"
#include "UISystems/ModalSystem.h"

#include "Sandbox/Modals/Modal.h"

void ModalSystem::Update()
{
	for (const auto& [modalId, modal] : s_modals)
	{
		modal->Update();
	}
}

void ModalSystem::RemoveModal(const UUID64& modalId)
{
	if (s_modals.contains(modalId))
	{
		s_modals.erase(modalId);
	}
}
