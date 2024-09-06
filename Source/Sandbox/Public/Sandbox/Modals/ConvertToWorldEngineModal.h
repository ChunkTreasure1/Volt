#pragma once

#include "Sandbox/Modals/Modal.h"

namespace Volt
{
	class Scene;
}

class ConvertToWorldEngineModal : public Modal
{
public:
	ConvertToWorldEngineModal(const std::string& stringId);
	~ConvertToWorldEngineModal();

	inline void SetCurrentScene(Weak<Volt::Scene> scene) { m_scene = scene; }

protected:
	void DrawModalContent() override;
	void OnClose() override;

protected:
	Weak<Volt::Scene> m_scene;
};
