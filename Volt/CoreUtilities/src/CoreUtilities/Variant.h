#pragma once

#include <cstdint>
#include <type_traits>

template<typename... T>
class Variant
{
public:
	Variant()
	{
		memset(m_buffer, 0, m_largestTypeSize);
	}

	~Variant() = default;

	template<typename F>
	F& Get()
	{
		static_assert(IsValidType<F>());

		F* value = nullptr;

		if (!m_first)
		{
			new (m_buffer) F();
			m_first = true;
		}

		value = reinterpret_cast<F*>(m_buffer);
		return *value;
	}

	template<typename F>
	const F& Get() const
	{
		static_assert(IsValidType<F>());

		const F* value = nullptr;

		if (!m_first)
		{
			new (const_cast<uint8_t*>(m_buffer)) F();
			const_cast<bool&>(m_first) = true;
		}

		value = reinterpret_cast<const F*>(m_buffer);
		return *value;
	}

private:
	static constexpr size_t FindLargestType()
	{
		size_t max = 0;
		([&]() 
		{
			if (max < sizeof(T))
			{
				max = sizeof(T);
			}
		}(), ...);

		return max;
	}

	template<typename F>
	static constexpr bool IsValidType()
	{
		bool valid = false;
		([&]()
		{
			valid |= std::is_same_v<F, T>;
		}(), ...);

		return valid;

		return true;
	}

	static constexpr size_t m_largestTypeSize = FindLargestType();
	uint8_t m_buffer[m_largestTypeSize];
	bool m_first = false;
};