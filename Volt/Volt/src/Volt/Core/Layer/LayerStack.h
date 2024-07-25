#pragma once

#include "Volt/Core/Base.h"



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

		Vector<Layer*> GetLayerStack() { return m_layers; }

		Vector<Layer*>::iterator begin() { return m_layers.begin(); }
		Vector<Layer*>::iterator end() { return m_layers.end(); }

	private:
		Vector<Layer*> m_layers;
		uint32_t m_lastInsertIndex = 0;
	};
}
