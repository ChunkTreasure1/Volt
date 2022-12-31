#pragma once
#include "HUD/Layer/Base/HUDLayerBase.h"

namespace HUD
{
	class LayerDebug : public HUD::LayerBase
	{
		//FUNCTIONS
	public:
		LayerDebug(Ref<Volt::SceneRenderer>& aSceneRenderer, std::string aLayerName);
		~LayerDebug() override;

		inline static LayerDebug& Get() { return *instance; }

	private:
		bool OnKeyEvent(Volt::KeyPressedEvent& e) override;

		//VARIABLES
	public:

	private:
		inline static LayerDebug* instance = nullptr;
	};
}