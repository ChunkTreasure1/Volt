#pragma once

#include <Wire/WireGUID.h>

#include "Volt/Scene/Entity.h"

namespace Volt
{
	class Entity;
	class Script
	{
	public:
		Script(Entity entity);
		virtual ~Script() = default;

		// This is called when the script is added to an entity
		virtual void OnAwake() {}

		// This is called when the script is removed from an entity
		virtual void OnDetach() {}

		// This is called when the scene starts playing
		virtual void OnStart() {}

		// This is called when the scene stops playing
		virtual void OnStop() {}

		// This is called every scene update
		virtual void OnUpdate(float) {}

		// This is called every physics update
		virtual void OnFixedUpdate(float) {}

		// This is called on every event
		virtual void OnEvent(Event&) {}

		virtual void OnCollisionEnter(Entity) {}
		virtual void OnCollisionExit(Entity) {}

		virtual void OnTriggerEnter(Entity, bool) {}
		virtual void OnTriggerExit(Entity, bool) {}

		virtual WireGUID GetGUID() = 0;

	protected:
		Entity myEntity;
	};
}