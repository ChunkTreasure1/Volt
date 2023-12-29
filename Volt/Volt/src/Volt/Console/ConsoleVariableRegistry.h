#pragma once

#include "Volt/Core/Base.h"

#include "Volt/Utility/StringUtility.h"

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
		
		virtual bool IsInteger() const = 0;
		virtual bool IsFloat() const = 0;
		virtual bool IsString() const = 0;
	};

	template<typename T>
	concept ValidConsoleVariableType = std::is_same_v<T, int32_t> || std::is_same_v<T, float> || std::is_same_v<T, std::string>;

	template<ValidConsoleVariableType T>
	class RegisteredConsoleVariable : public RegisteredConsoleVariableBase
	{
	public:
		RegisteredConsoleVariable(const std::string& variableName, const T& defaultValue, std::string_view description);

		[[nodiscard]] const void* Get() const override;
		void Set(const void* value) override;

		[[nodiscard]] inline std::string_view GetName() const override { return m_variableName; }
		[[nodiscard]] inline std::string_view GetDescription() const override { return m_description; }

		[[nodiscard]] inline constexpr bool IsInteger() const override { return std::is_integral_v<T>; }
		[[nodiscard]] inline constexpr bool IsFloat() const override { return std::is_floating_point_v<T>; }
		[[nodiscard]] inline constexpr bool IsString() const override { return std::is_same_v<T, std::string>; }

	private:
		T m_value;
		std::string m_variableName;
		std::string_view m_description;
	};

	template<ValidConsoleVariableType T>
	class ConsoleVariable
	{
	public:
		ConsoleVariable(std::string_view variableName, const T& defaultValue, std::string_view description);

		const T& GetValue() const { return *reinterpret_cast<const T*>(m_variableReference->Get()); }
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

	template<ValidConsoleVariableType T>
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
		template<ValidConsoleVariableType T>
		static Weak<RegisteredConsoleVariable<T>> RegisterVariable(std::string_view variableName, const T& defaultValue, std::string_view description);

		template<ValidConsoleVariableType T>
		static Weak<RegisteredConsoleVariable<T>> FindVariable(const std::string& variableName);

		inline static Weak<RegisteredConsoleVariableBase> GetVariable(const std::string& variableName);
		inline static bool VariableExists(const std::string& variableName);

		static std::unordered_map<std::string, Ref<RegisteredConsoleVariableBase>>& GetRegisteredVariables() { return s_registeredVariables; }

	private:
		ConsoleVariableRegistry() = delete;

		inline static std::unordered_map<std::string, Ref<RegisteredConsoleVariableBase>> s_registeredVariables;
	};

	inline bool ConsoleVariableRegistry::VariableExists(const std::string& variableName)
	{
		std::string tempVarName = ::Utility::ToLower(std::string(variableName));
		return s_registeredVariables.contains(tempVarName);
	}

	template<ValidConsoleVariableType T>
	inline RegisteredConsoleVariable<T>::RegisteredConsoleVariable(const std::string& variableName, const T& defaultValue, std::string_view description)
		: m_value(defaultValue), m_variableName(variableName), m_description(description)
	{
	}

	template<ValidConsoleVariableType T>
	inline const void* RegisteredConsoleVariable<T>::Get() const
	{
		return reinterpret_cast<const void*>(&m_value);
	}

	template<ValidConsoleVariableType T>
	inline void RegisteredConsoleVariable<T>::Set(const void* value)
	{
		m_value = *reinterpret_cast<const T*>(value);
	}

	template<ValidConsoleVariableType T>
	inline ConsoleVariable<T>::ConsoleVariable(std::string_view variableName, const T& defaultValue, std::string_view description)
	{
		m_variableReference = ConsoleVariableRegistry::RegisterVariable<T>(variableName, defaultValue, description);
	}

	template<ValidConsoleVariableType T>
	inline Weak<RegisteredConsoleVariable<T>> ConsoleVariableRegistry::RegisterVariable(std::string_view variableName, const T& defaultValue, std::string_view description)
	{
		std::string tempVarName = ::Utility::ToLower(std::string(variableName));

		Ref<RegisteredConsoleVariable<T>> consoleVariable = CreateRef<RegisteredConsoleVariable<T>>(tempVarName, defaultValue, description);

		VT_CORE_ASSERT(!s_registeredVariables.contains(tempVarName), "Command variable with name already registered!");
		s_registeredVariables[tempVarName] = consoleVariable;

		return consoleVariable;
	}
	
	template<ValidConsoleVariableType T>
	inline Weak<RegisteredConsoleVariable<T>> ConsoleVariableRegistry::FindVariable(const std::string& variableName)
	{
		const std::string tempVarName = ::Utility::ToLower(variableName);

		if (s_registeredVariables.contains(tempVarName))
		{
			return s_registeredVariables.at(tempVarName);
		}

		return Weak<RegisteredConsoleVariable<T>>();
	}

	template<ValidConsoleVariableType T>
	inline ConsoleVariableRef<T>::ConsoleVariableRef(std::string_view variableName)
	{
		m_variableReference = ConsoleVariableRegistry::FindVariable(variableName);
		VT_CORE_ASSERT(m_variableReference, "Variable with name not found!");
	}

	inline Weak<RegisteredConsoleVariableBase> ConsoleVariableRegistry::GetVariable(const std::string& variableName)
	{
		const std::string tempVarName = ::Utility::ToLower(variableName);
		return s_registeredVariables.at(tempVarName);
	}
}
