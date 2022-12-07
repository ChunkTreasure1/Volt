#pragma once

#include "Volt/Events/Event.h"

namespace Volt
{
	class Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnEvent(Event&) {}
	};
}