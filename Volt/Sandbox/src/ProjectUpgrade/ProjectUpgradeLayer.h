#pragma once

#include <Volt/Core/Layer/Layer.h>

namespace Volt
{
	class AppImGuiUpdateEvent;
}

class ProjectUpgradeLayer : public Volt::Layer
{
public:
	ProjectUpgradeLayer();
	~ProjectUpgradeLayer() override;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Volt::Event& e) override;

private:
	void DrawUpgradeUI();
	bool OnImGuiUpdateEvent(Volt::AppImGuiUpdateEvent& e);
};
