#pragma once
#include "Circuit/CircuitCoreDefines.h"
#include "Circuit/Events/CircuitEventTypes.h"

#define LISTEN_EVENT_CLASS_TYPE(type) CIRCUIT_API static Circuit::CircuitListenEventType GetStaticType() {return Circuit::##type;}\
										CIRCUIT_API virtual Circuit::CircuitListenEventType GetEventType() const override { return GetStaticType(); }\
										CIRCUIT_API virtual const char* GetName() const override { return #type; }
namespace Circuit
{

	class ListenEvent
	{
	public:
		virtual ~ListenEvent() = default;


		CIRCUIT_API virtual Circuit::CircuitListenEventType GetEventType() const = 0;
		CIRCUIT_API virtual const char* GetName() const = 0;
		CIRCUIT_API virtual std::string ToString() const { return GetName(); }
	protected:
		ListenEvent() {};
	};
}
