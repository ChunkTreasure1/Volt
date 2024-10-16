#pragma once

#include <Volt-Core/Layer/Layer.h>

#include <EventSystem/EventListener.h>

namespace Volt
{
	class AppImGuiUpdateEvent;
}

class ProjectUpgradeLayer : public Volt::Layer, public Volt::EventListener
{
public:
	ProjectUpgradeLayer();
	~ProjectUpgradeLayer() override;

	void OnAttach() override;
	void OnDetach() override;

private:
	void DrawUpgradeUI();
	bool OnImGuiUpdateEvent(Volt::AppImGuiUpdateEvent& e);
};
