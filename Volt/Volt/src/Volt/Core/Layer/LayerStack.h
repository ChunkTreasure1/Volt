#pragma once

#include "Volt/Core/Base.h"

#include <vector>

namespace Volt
{
	class Layer;
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		void PopLast();
		Layer* GetLastLayer();

		void Clear();

		std::vector<Layer*> GetLayerStack() { return m_layers; }

		std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_layers.end(); }

	private:
		std::vector<Layer*> m_layers;
		uint32_t m_lastInsertIndex = 0;
	};
}
