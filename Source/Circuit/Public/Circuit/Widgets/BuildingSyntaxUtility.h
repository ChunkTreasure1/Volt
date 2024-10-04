#pragma once

#include <CoreUtilities/Delegates/Attribute.h>

#define CreateWidget(InWidgetType) \
	CircuitDeclare<InWidgetType>() <<= InWidgetType::Arguments()

#define CIRCUIT_BEGIN_ARGS(InWidgetType) \
	public: \
	struct Arguments : public CircuitBaseArgsTemplate<InWidgetType>\
	{ \
		using WidgetArgumentsType = Arguments; \
		using WidgetType = InWidgetType; \
		Arguments()

#define CIRCUIT_END_ARGS() \
	};

#define CIRCUIT_ARGUMENT_VARIABLE(ArgType, ArgName) \
	ArgType _##ArgName

#define CIRCUIT_ARGUMENT_FUNCTION(ArgType, ArgName) \
	WidgetArgumentsType& ArgName(ArgType ArgName) \
	{ \
		_##ArgName = ArgName; \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	}

#define CIRCUIT_ARGUMENT(ArgType, ArgName) \
	CIRCUIT_ARGUMENT_VARIABLE(ArgType, ArgName); \
	CIRCUIT_ARGUMENT_FUNCTION(ArgType, ArgName)

#define CIRCUIT_ATTRIBUTE_VARIABLE(AttrType, AttrName)\
	Volt::Attribute<AttrType> _##AttrName

#define CIRCUIT_ATTRIBUTE_FUNCTION(AttrType, AttrName)\
	WidgetArgumentsType& ##AttrName(Volt::Attribute<AttrType> attribute)\
	{\
		_##AttrName = std::move(attribute);\
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	}\
	\
	template< typename... VarTypes > \
	WidgetArgumentsType& AttrName##_Static(std::type_identity< typename Volt::Attribute< AttrType >::Getter::template FnPtr<VarTypes...> > func, VarTypes... vars)	\
	{ \
		_##AttrName = std::move(Volt::Attribute< AttrType >::Create(std::move(Volt::Attribute< AttrType >::Getter::CreateStatic(func, std::forward(vars)...)))); \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	} \
	template<typename FunctorType, typename... LambdaParamTypes> \
	WidgetArgumentsType& AttrName##_Lambda(FunctorType&& functor, LambdaParamTypes&&... params)	\
	{ \
		_##AttrName = std::move(Volt::Attribute< AttrType >::Create(std::move(Volt::Attribute< AttrType >::Getter::CreateLambda(functor, std::forward(params)...)))); \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	} \
	template <typename UserClass, typename... RawFnParamTypes> \
	WidgetArgumentsType& AttrName##_Raw(const UserClass* userObject, Volt::Attribute< AttrType >::Getter::template ConstMemberFnPtr<UserClass> func, RawFnParamTypes&&... params)	\
	{ \
		_##AttrName = std::move(Volt::Attribute< AttrType >::Create(std::move(Volt::Attribute< AttrType >::Getter::CreateRaw(userObject, func,  std::forward(params)...)))); \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	} \


#define CIRCUIT_ATTRIBUTE(AttrType, AttrName)\
	CIRCUIT_ATTRIBUTE_VARIABLE(AttrType, AttrName); \
	CIRCUIT_ATTRIBUTE_FUNCTION(AttrType, AttrName); \

#define CIRCUIT_EVENT(DelegateType, EventName) \
	DelegateType _##EventName; \
	WidgetArgumentsType& EventName(const DelegateType& inDelegate) \
	{ \
		_##EventName = inDelegate; \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	} \
	template< typename... VarTypes > \
	WidgetArgumentsType& EventName##_Static(std::type_identity< typename DelegateType:: template FnPtr<VarTypes...> > func, VarTypes... vars)	\
	{ \
		_##EventName = DelegateType::CreateStatic(func, std::forward(vars)...); \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	} \
	template<typename FunctorType, typename... LambdaParamTypes> \
	WidgetArgumentsType& EventName##_Lambda(FunctorType&& functor, LambdaParamTypes&&... params)	\
	{ \
		_##EventName = DelegateType::CreateLambda(functor, std::forward(params)...); \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	} \
	template <typename UserClass, typename... RawFnParamTypes> \
	WidgetArgumentsType& EventName##_Raw(const UserClass* userObject, DelegateType::template ConstMemberFnPtr<UserClass> func, RawFnParamTypes&&... params)	\
	{ \
		_##EventName = DelegateType::CreateRaw(userObject, func, std::forward(params)...); \
		return static_cast<WidgetArgumentsType*>(this)->Me(); \
	} \

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
		m_Widget->CalculateBounds();
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
