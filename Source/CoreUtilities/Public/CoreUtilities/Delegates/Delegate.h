#pragma once

#include "CoreUtilities/Delegates/DelegateInstance.h"

namespace Volt
{
	template<typename ReturnType, typename... ParamTypes>
	class Delegate
	{

	};

	template<typename ReturnType, typename... ParamTypes>
	class Delegate<ReturnType(ParamTypes...)>
	{
		using FnType = ReturnType(ParamTypes...);

		DelegateInstance<FnType>* m_delegateInstance = nullptr;


	public:
		~Delegate()
		{
			Unbind();
		}
	private:
		template<typename DelegateInstanceType, typename... DelegateInstanceParams>
		void CreateDelegateInstance(DelegateInstanceParams&&... params)
		{
			/*DelegateInstance<FnType>* delegateInstance = m_delegateInstance;
			if (DelegateInstance)
			{
				DelegateInstance->~IDelegateInstance();
			}*/

			m_delegateInstance = new DelegateInstanceType(std::forward<DelegateInstanceParams>(params)...);
		}

	public:
		bool IsBound()
		{
			return m_delegateInstance != nullptr && m_delegateInstance->IsSafeToExecute();
		}
	public:
		template<typename FunctorType, typename... LambdaParamTypes>
		[[nodiscard]] static Delegate<FnType> CreateLambda(FunctorType&& functor, LambdaParamTypes&&... params)
		{
			Delegate<FnType> result;
			result.template CreateDelegateInstance<FunctorDelegateInstance<FnType, typename std::remove_reference<FunctorType>::type, std::decay_t<LambdaParamTypes>...>>(std::forward<FunctorType>(functor), std::forward<LambdaParamTypes>(params)...);
			return result;
		}

		template< typename... StaticFnParamTypes>
		[[nodiscard]] static Delegate<FnType> CreateStatic(typename std::type_identity<ReturnType(*)(ParamTypes..., std::decay_t<StaticFnParamTypes>...)>::type func, StaticFnParamTypes&&... params)
		{
			Delegate<FnType> result;
			result.template CreateDelegateInstance<StaticDelegateInstance<FnType, std::decay_t<StaticFnParamTypes>...>>(func, std::forward<StaticFnParamTypes>(params)...);
			return result;
		}

		template <typename UserClass, typename... RawFnParamTypes>
		[[nodiscard]] inline static Delegate<FnType> CreateRaw(UserClass* userObject, typename MemFnPtrType<false, UserClass, ReturnType(ParamTypes..., std::decay_t<RawFnParamTypes>...)>::Type func, RawFnParamTypes&&... params)
		{
			static_assert(!std::is_const_v<UserClass>, "Attempting to bind a delegate with a const object pointer and non-const member function.");

			Delegate<FnType> result;
			result.template CreateDelegateInstance<RawFunctionDelegateInstance<false, UserClass, FnType, std::decay_t<RawFnParamTypes>...>>(userObject, func, std::forward<RawFnParamTypes>(params)...);
			return result;
		}

		template <typename UserClass, typename... RawFnParamTypes>
		[[nodiscard]] inline static Delegate<FnType> CreateRaw(const UserClass* userObject, typename MemFnPtrType<true, UserClass, ReturnType(ParamTypes..., std::decay_t<RawFnParamTypes>...)>::Type func, RawFnParamTypes&&... params)
		{
			Delegate<FnType> result;
			result.template CreateDelegateInstance<RawFunctionDelegateInstance<true, UserClass, FnType, std::decay_t<RawFnParamTypes>...>>(userObject, func, std::forward<RawFnParamTypes>(params)...);
			return result;
		}


	public:
		ReturnType Execute(ParamTypes... params) const
		{
			return m_delegateInstance->Execute(std::forward<ParamTypes>(params)...);
		}


		template <
			// This construct is intended to disable this function when ReturnType != void.
			typename DummyRetValType = ReturnType,
			std::enable_if_t<std::is_void<DummyRetValType>::value>* = nullptr
		>
		bool ExecuteIfBound(ParamTypes... params) const
		{
			if (m_delegateInstance)
			{
				m_delegateInstance->Execute(std::forward<ParamTypes>(params)...);
				return true;
			}
			return false;
		}

		void Unbind()
		{
			delete m_delegateInstance;
			m_delegateInstance = nullptr;
		}
	};
};
