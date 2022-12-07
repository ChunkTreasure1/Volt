#pragma once

namespace Volt{
	struct EventMessage{
		class Observer;
		void* ptrVoid = nullptr;
		Observer* ptrObserver = nullptr;
	};

	enum class eEvent{
		TEST_EVENT
	};
}