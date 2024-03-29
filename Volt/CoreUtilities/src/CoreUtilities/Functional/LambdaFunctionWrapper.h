#pragma once

template<typename CT, typename... A>
struct LambdaFunctionWrapper : public LambdaFunctionWrapper<decltype(&CT::operator())(A...)>
{ };

template<typename C>
struct LambdaFunctionWrapper<C>
{
public:
	LambdaFunctionWrapper()
		: m_isValid(false)
	{}

	LambdaFunctionWrapper(const C& func)
		: m_function(func), m_isValid(true)
	{ }

	inline const bool IsValid() const { return m_isValid; }

	template<typename... Args>
	typename std::result_of<C(Args...)>::type operator()(Args... a)
	{
		return m_function.operator()(a...);
	}

	template<typename... Args>
	typename std::result_of<const C(Args...)>::type operator()(Args... a) const
	{
		return m_function.operator()(a...);
	}

	void operator=(const LambdaFunctionWrapper<C>& other)
	{
		m_function = other.m_function;
		m_isValid = true;
	}

	void operator=(const C& other)
	{
		m_function = other;
		m_isValid = true;
	}

	bool operator!() const
	{
		return !m_isValid;
	}

	bool explicit operator() const
	{
		return m_isValid;
	}

private:
	C m_function;
	bool m_isValid;
};
