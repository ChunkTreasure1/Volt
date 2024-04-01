#pragma once
#include "Delegates/Delegate.h"
#include "Delegates/DelegateDeclarationHelpers.h"

template<typename T>
class Observer
{

public:
	DECLARE_DELEGATE(ObserverOnChange, void, T);

	Observer(T* value)
		: m_ValuePtr(value)
	{
	}

	// inplace operators
#define X(op) \
    template<typename Rhs> \
    Observer& operator op(Rhs const& rhs) noexcept \
        requires requires(T& a, Rhs const& b) { a op b; } \
    { \
		(*m_ValuePtr) op rhs; \
		m_ObserverOnChange.Broadcast(*m_ValuePtr); \
        return *this; \
    }

	X(= );
	X(+= );
	X(-= );
	X(*= );
	X(/= );
	X(%= );
	X(&= );
	X(|= );
	X(^= );
	X(<<= );
	X(>>= );
#undef X
	const T& GetValue() const
	{
		return *m_ValuePtr;
	}

	const T& Get() const { return *m_ValuePtr; }
	T operator*() { return *m_ValuePtr; }

	ObserverOnChange& GetOnChangeDelegate()
	{
		return m_ObserverOnChange;
	}

private:
	T* m_ValuePtr;
	ObserverOnChange m_ObserverOnChange;

};
