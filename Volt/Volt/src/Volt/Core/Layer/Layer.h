#pragma once

namespace Volt
{
	class Event;
	class Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnEvent(Event&) {}
	};
}
