#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/UUID.h"

namespace Volt
{
	using GCHandle = void*;

	class MonoScriptClass;
	class MonoCoreInstance
	{
	public:
		MonoCoreInstance(Ref<MonoScriptClass> monoClass);
		~MonoCoreInstance();

		inline const Ref<MonoScriptClass> GetClass() const { return myMonoClass; }
		inline const GCHandle GetHandle() const { return myHandle; }

		void CallMethod(const std::string& methodName);

	private:
		Ref<MonoScriptClass> myMonoClass;

		UUID64 myUUID{};
		GCHandle myHandle = nullptr;
	};
}
