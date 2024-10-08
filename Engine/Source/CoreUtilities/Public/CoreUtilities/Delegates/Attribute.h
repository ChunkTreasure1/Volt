#pragma once
#include "CoreUtilities/Delegates/Delegate.h"

namespace Volt
{
	template<typename T>
	class Attribute
	{
	public:
		DECLARE_DELEGATE_RetVal(T, Getter);

		Attribute()
			: m_value(T()),
			m_getter()
		{
		}

		Attribute(T value)
			: m_value(value),
			m_getter()
		{
		}


	public:
		T Get() const
		{
			if (m_getter.IsBound())
			{
				m_value = m_getter.Execute();
			}
			return m_value;
		}

		void Set(const T& value)
		{
			if (m_getter.IsBound())
			{
				m_getter.Unbind();
			}
			m_value = value;
		}
	public:
		void Bind(const Getter& getter)
		{
			m_getter = getter;
		}

		template<typename FunctorType, typename... LambdaParamTypes>
		void BindLambda(FunctorType&& functor, LambdaParamTypes&&... params)
		{
			m_getter.BindLambda(functor, params...);
		}

		template< typename... StaticFnParamTypes>
		void BindStatic(std::type_identity<typename Getter::template FnPtr<StaticFnParamTypes...>>::type func, StaticFnParamTypes... params)
		{
			m_getter.BindStatic(func, params...);
		}

		template <typename UserClass, typename... RawFnParamTypes>
		void BindRaw(const UserClass* userObject, Getter::template ConstMemberFnPtr<UserClass> func, RawFnParamTypes&&... params)
		{
			m_getter.BindRaw(userObject, func, params);
		}

	public:
		static Attribute<T> Create(const Getter& getter)
		{
			return Attribute<T>(getter);
		}
		static Attribute<T> Create(Getter&& getter)
		{
			return Attribute<T>(std::move(getter));
		}
		template<typename FunctorType, typename... LambdaParamTypes>
		static Attribute<T> CreateLambda(FunctorType&& functor, LambdaParamTypes&&... params)
		{
			return Attribute<T>(Getter::CreateLambda(functor, std::forward<LambdaParamTypes>(params)...));
		}

		template< typename... StaticFnParamTypes>
		static Attribute<T> CreateStatic(std::type_identity<typename Getter::template FnPtr<StaticFnParamTypes...>>::type func, StaticFnParamTypes... params)
		{
			return Attribute<T>(Getter::CreateStatic(func, std::forward<StaticFnParamTypes>(params)...));
		}

		template <typename UserClass, typename... RawFnParamTypes>
		static Attribute<T> CreateRaw(const UserClass* userObject, Getter::template ConstMemberFnPtr<UserClass> func, RawFnParamTypes&&... params)
		{
			return Attribute<T>(Getter::CreateRaw(userObject, func, std::forward<RawFnParamTypes>(params)...));
		}

	private:
		Attribute<T>(const Getter& getter)
			: m_value(),
			m_getter(getter)
		{
		}

		mutable T m_value;
		Getter m_getter;

	};
}
