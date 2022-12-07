#include "vtpch.h"
#include "LayerStack.h"

#include "Volt/Core/Layer/Layer.h"

namespace Volt
{
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		Clear();
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_layers.emplace(m_layers.begin() + m_lastInsertIndex, layer);
		m_lastInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.begin() + m_lastInsertIndex, layer);
		if (it != m_layers.begin() + m_lastInsertIndex)
		{
			layer->OnDetach();
			m_layers.erase(it);
			m_lastInsertIndex--;
			delete layer;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(m_layers.begin() + m_lastInsertIndex, m_layers.end(), overlay);
		if (it != m_layers.end())
		{
			overlay->OnDetach();
			m_layers.erase(it);
			delete overlay;
		}
	}

	void LayerStack::Clear()
	{
		for (Layer* layer : m_layers)
		{
			layer->OnDetach();
			delete layer;
		}

		m_layers.clear();
	}
}