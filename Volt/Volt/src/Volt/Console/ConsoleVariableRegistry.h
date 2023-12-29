#pragma once

#include "Volt/Core/Base.h"

#include <unordered_map>
#include <string_view>

namespace Volt
{
	class RegisteredConsoleVariableBase
	{
	public:
		virtual const void* Get() const = 0;
		virtual void Set(const void* value) = 0;

		virtual std::string_view GetName() const = 0;
		virtual std::string_view GetDescription() const = 0;
	};

	template<typename T> 
	class RegisteredConsoleVariable : public RegisteredConsoleVariableBase
	{
	public:
		RegisteredConsoleVariable(std::string_view variableName, const T& defaultValue, std::string_view description);

		const void* Get() const override;
		void Set(const void* value) override;

		std::string_view GetName() const override { return m_variableName; }
		std::string_view GetDescription() const override { return m_description; }

	private:
		T m_value;
		std::string_view m_variableName;
		std::string_view m_description;
	};

	template<typename T>
	class ConsoleVariable
	{
	public:
		ConsoleVariable(std::string_view variableName, const T& defaultValue, std::string_view description);

		const T& GetValue() const { return *reinterpret_cast<T*>(m_variableReference->Get()); }
		void SetValue(const T& value) { m_variableReference->Set(&value); }

		T& operator=(const T& other)
		{
			if (this == &other)
			{
				return GetValue();
			}

			SetValue(other);
		}

	private:
		Weak<RegisteredConsoleVariable<T>> m_variableReference;
	};

	template<typename T>
	class ConsoleVariableRef
	{
	public:
		ConsoleVariableRef(std::string_view variableName);

		const T& GetValue() const { return *reinterpret_cast<T*>(m_variableReference->Get()); }
		void SetValue(const T& value) { m_variableReference->Set(&value); }

		T& operator=(const T& other)
		{
			if (this == &other)
			{
				return GetValue();
			}

			SetValue(other);
		}

	private:
		Weak<RegisteredConsoleVariable<T>> m_variableReference;
	};

	class ConsoleVariableRegistry
	{
	public:
		template<typename T>
		static Weak<RegisteredConsoleVariable<T>> RegisterVariable(std::string_view variableName, const T& defaultValue, std::string_view description);

		template<typename T>
		static Weak<RegisteredConsoleVariable<T>> FindVariable(std::string_view variableName);

		static std::unordered_map<std::string_view, Ref<RegisteredConsoleVariableBase>>& GetRegisteredVariables() { return s_registeredVariables; }

	private:
		inline static std::unordered_map<std::string_view, Ref<RegisteredConsoleVariableBase>> s_registeredVariables;
	};

	template<typename T>
	inline RegisteredConsoleVariable<T>::RegisteredConsoleVariable(std::string_view variableName, const T& defaultValue, std::string_view description)
		: m_value(defaultValue)
	{
	}

	template<typename T>
	inline const void* RegisteredConsoleVariable<T>::Get() const
	{
		return reinterpret_cast<const void*>(&m_value);
	}

	template<typename T>
	inline void RegisteredConsoleVariable<T>::Set(const void* value)
	{
		m_value = *reinterpret_cast<const T*>(value);
	}

	template<typename T>
	inline ConsoleVariable<T>::ConsoleVariable(std::string_view variableName, const T& defaultValue, std::string_view description)
	{
		m_variableReference = ConsoleVariableRegistry::RegisterVariable<T>(variableName, defaultValue, description);
	}

	template<typename T>
	inline Weak<RegisteredConsoleVariable<T>> ConsoleVariableRegistry::RegisterVariable(std::string_view variableName, const T& defaultValue, std::string_view description)
	{
		Ref<RegisteredConsoleVariable<T>> consoleVariable = CreateRef<RegisteredConsoleVariable<T>>(variableName, defaultValue, description);

		VT_CORE_ASSERT(!s_registeredVariables.contains(variableName), "Command variable with name already registered!");
		s_registeredVariables[variableName] = consoleVariable;

		return consoleVariable;
	}
	
	template<typename T>
	inline Weak<RegisteredConsoleVariable<T>> ConsoleVariableRegistry::FindVariable(std::string_view variableName)
	{
		if (s_registeredVariables.contains(variableName))
		{
			return s_registeredVariables.at(variableName);
		}

		return Weak<RegisteredConsoleVariable<T>>();
	}

	template<typename T>
	inline ConsoleVariableRef<T>::ConsoleVariableRef(std::string_view variableName)
	{
		m_variableReference = ConsoleVariableRegistry::FindVariable(variableName);
		VT_CORE_ASSERT(m_variableReference, "Variable with name not found!");
	}
}
