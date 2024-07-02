#pragma once
#include "Circuit/CircuitCoreDefines.h"

#define CreateWidget(InWidgetType) \
	CircuitDeclare<InWidgetType>() <<= InWidgetType::Arguments()


#define CIRCUIT_BEGIN_ARGS(InWidgetType) \
	public: \
	struct Arguments : public CircuitBaseArgsTemplate<InWidgetType>\
	{ \
		using WidgetArgumentsType = Arguments; \
		using WidgetType = InWidgetType; \
		CIRCUIT_API Arguments()

#define CIRCUIT_END_ARGS() \
	};


#define CIRCUIT_ARGUMENT_VARIABLE(ArgType, ArgName) \
	ArgType _##ArgName

#define CIRCUIT_ARGUMENT_FUNCTION(ArgType, ArgName) \
	CIRCUIT_API WidgetArgumentsType& ArgName(ArgType ArgName) \
	{ \
		_##ArgName = ArgName; \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	}

#define CIRCUIT_ARGUMENT(ArgType, ArgName) \
	CIRCUIT_ARGUMENT_VARIABLE(ArgType, ArgName); \
	CIRCUIT_ARGUMENT_FUNCTION(ArgType, ArgName)



template<typename WidgetType>
struct CircuitDeclare
{
	CircuitDeclare()
	{
		m_Widget = std::make_unique<WidgetType>();
	}
	std::unique_ptr<WidgetType>& operator<<=(const typename WidgetType::Arguments::WidgetArgumentsType& args)
	{
		m_Widget->BuildBaseArgs(args);
		m_Widget->Build(args);
		return m_Widget;
	}
	std::unique_ptr<WidgetType> m_Widget;
};

/** Base class for named arguments. Provides settings necessary for all widgets. */
struct CircuitBaseArgs
{
	CircuitBaseArgs() = default;

	CIRCUIT_ARGUMENT_VARIABLE(float, X);
	CIRCUIT_ARGUMENT_VARIABLE(float, Y);
};


/** Base class for named arguments. Provides settings necessary for all widgets. */
template<typename InWidgetType>
struct CircuitBaseArgsTemplate : public CircuitBaseArgs
{
	using WidgetArgumentsType = InWidgetType::Arguments;
	using WidgetType = InWidgetType;
	
	CIRCUIT_ARGUMENT_FUNCTION(float, X);
	CIRCUIT_ARGUMENT_FUNCTION(float, Y);
	

	/** Used by the named argument pattern as a safe way to 'return *this' for call-chaining purposes. */
	WidgetArgumentsType& Me()
	{
		return *(static_cast<WidgetArgumentsType*>(this));
	}
};
