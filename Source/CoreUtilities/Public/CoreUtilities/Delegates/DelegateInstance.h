#pragma once

#include "CoreUtilities/Delegates/DelegateHandle.h"
#include "CoreUtilities/GUIDUtilities.h"

#include <tuple>

namespace Volt
{
	template<typename ReturnType, typename... ParamTypes>
	class DelegateInstance;

	template <typename FnType, typename FunctorType, typename... VarTypes>
	class FunctorDelegateInstance;

	template <typename FnType, typename... VarTypes>
	class StaticDelegateInstance;

	template <bool Const, class UserClass, typename FnType, typename... VarTypes>
	class RawFunctionDelegateInstance;


	template<typename ReturnType, typename... ParamTypes>
	class DelegateInstance<ReturnType(ParamTypes...)>
	{
	public:
		template<typename... InParamTypes>
		explicit DelegateInstance(InParamTypes&&... params)
			: m_paramTypes(std::forward<InParamTypes>(params)...)
			, m_handle(GUIDUtilities::GenerateGUID())
		{
		}

		virtual bool IsSafeToExecute() const = 0;
		virtual ReturnType Execute(ParamTypes...) const = 0;
		virtual bool ExecuteIfSafe(ParamTypes...) const = 0;

		virtual DelegateInstance<ReturnType(ParamTypes...)>* CreateCopy() const = 0;
	protected:
		std::tuple<ParamTypes...> m_paramTypes;

		DelegateHandle m_handle;
	};


	template <typename ReturnType, typename... ParamTypes, typename FunctorType, typename... VarTypes>
	class FunctorDelegateInstance<ReturnType(ParamTypes...), FunctorType, VarTypes...> : public DelegateInstance<ReturnType(ParamTypes...), VarTypes...>
	{
		static_assert(std::is_same_v<FunctorType, typename std::remove_reference_t<FunctorType>>, "FunctorType cannot be a reference");

		using Base = DelegateInstance<ReturnType(ParamTypes...), VarTypes...>;
	public:
		template <typename InFunctorType, typename... InVarTypes>
		explicit FunctorDelegateInstance(InFunctorType&& inFunctor, InVarTypes&&... vars)
			:Base(std::forward(vars)...)
			, m_functor(std::forward<InFunctorType>(inFunctor))
		{
		}

		bool IsSafeToExecute() const override final
		{
			//functors are always considered safe to execute
			return true;
		}
		
		ReturnType Execute(ParamTypes... params) const override final
		{
			return m_functor(std::forward<ParamTypes>(params)...);
		}

		bool ExecuteIfSafe(ParamTypes...params) const override final
		{
			//functors are always considered safe to execute
			m_functor(std::forward<ParamTypes>(params)...);

			return true;
		}

		DelegateInstance<ReturnType(ParamTypes...)>* CreateCopy() const override
		{
			return new FunctorDelegateInstance(*this);
		}
	private:
		mutable std::remove_const_t<FunctorType> m_functor;
	};

	template <typename ReturnType, typename... ParamTypes, typename... VarTypes>
	class StaticDelegateInstance<ReturnType(ParamTypes...), VarTypes...> : public DelegateInstance<ReturnType(ParamTypes...), VarTypes...>
	{
		using Base = DelegateInstance<ReturnType(ParamTypes...), VarTypes...>;
		using FnPtr = ReturnType(*)(ParamTypes..., VarTypes...);


	public:
		template <typename... InVarTypes>
		explicit StaticDelegateInstance(FnPtr functionPtr, InVarTypes&&... vars)
			:Base(std::forward(vars)...)
			, m_staticFunctionPtr(functionPtr)
		{
			VT_ASSERT(m_staticFunctionPtr != nullptr);
		}

		bool IsSafeToExecute() const override final
		{
			//static functions are always safe to execute
			return true;
		}


		ReturnType Execute(ParamTypes... params) const override final
		{
			VT_ASSERT(m_staticFunctionPtr != nullptr);

			return m_staticFunctionPtr(std::forward<ParamTypes>(params)...);
		}

		bool ExecuteIfSafe(ParamTypes... params) const override final
		{
			VT_ASSERT(m_staticFunctionPtr != nullptr);

			m_staticFunctionPtr(std::forward<ParamTypes>(params)...);

			return true;
		}

		DelegateInstance<ReturnType(ParamTypes...)>* CreateCopy() const override
		{
			return new StaticDelegateInstance(*this);
		}
	private:
		FnPtr m_staticFunctionPtr;
	};


	template <bool Const, typename Class, typename FnType>
	struct MemFnPtrType;

	template <typename Class, typename ReturnType, typename... ParamTypes>
	struct MemFnPtrType < false, Class, ReturnType(ParamTypes...)>
	{
		typedef ReturnType(Class::* Type)(ParamTypes...);
	};

	template <typename Class, typename ReturnType, typename... ParamTypes>
	struct MemFnPtrType < true, Class, ReturnType(ParamTypes...)>
	{
		typedef ReturnType(Class::* Type)(ParamTypes...) const;
	};

	template <bool Const, class UserClass, typename ReturnType, typename... ParamTypes, typename... VarTypes >
	class RawFunctionDelegateInstance<Const, UserClass, ReturnType(ParamTypes...), VarTypes...> : public DelegateInstance<ReturnType(ParamTypes...), VarTypes...>
	{
		using Base = DelegateInstance<ReturnType(ParamTypes...), VarTypes...>;
		using FnPtr = typename MemFnPtrType<Const, UserClass, ReturnType(ParamTypes..., VarTypes...)>::Type;

	public:
		template <typename... InVarTypes>
		explicit RawFunctionDelegateInstance(UserClass* userObject, FnPtr functionPtr,InVarTypes&&... vars)
			:Base(std::forward(vars)...)
			, m_userObject(userObject)
			, m_functionPtr(functionPtr)
		{
			VT_ASSERT(userObject != nullptr && functionPtr != nullptr);
		}

		bool IsSafeToExecute() const override final
		{
			//Impossible to know if it's safe to execute, but trust the user here
			return true;
		}

		ReturnType Execute(ParamTypes... params) const override final
		{
			using MutableUserClass = std::remove_const_t<UserClass>;

			MutableUserClass* mutableUserObject = const_cast<MutableUserClass*>(m_userObject);
			return ((*mutableUserObject).*m_functionPtr)(std::forward<ParamTypes>(params)...);
		}

		bool ExecuteIfSafe(ParamTypes...params) const override final
		{
			using MutableUserClass = std::remove_const_t<UserClass>;

			MutableUserClass* mutableUserObject = const_cast<MutableUserClass*>(m_userObject);
			((*mutableUserObject).*m_functionPtr)(std::forward<ParamTypes>(params)...);
			return true;
		}

		DelegateInstance<ReturnType(ParamTypes...)>* CreateCopy() const override
		{
			return new RawFunctionDelegateInstance(*this);
		}
	private:
		UserClass* m_userObject;

		FnPtr m_functionPtr;
	};
}
