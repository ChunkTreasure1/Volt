#pragma once

#include <cstdint>

namespace Volt
{
	class ResourceHandle
	{
	public:
		// Constructors
		constexpr ResourceHandle() : value(0) {}
		constexpr explicit ResourceHandle(uint32_t val) : value(val) {}
		constexpr explicit ResourceHandle(size_t val) : value(static_cast<uint32_t>(val)) {}

		inline const uint32_t Get() const { return value; }

		// Overloaded operators
		ResourceHandle& operator++()
		{ // Prefix increment
			++value;
			return *this;
		}

		ResourceHandle operator++(int)
		{ // Postfix increment
			ResourceHandle temp(*this);
			++value;
			return temp;
		}

		ResourceHandle& operator--()
		{ // Prefix decrement
			--value;
			return *this;
		}

		ResourceHandle operator--(int)
		{ // Postfix decrement
			ResourceHandle temp(*this);
			--value;
			return temp;
		}

		operator uint32_t()
		{
			return value;
		}

		// Binary operators
		friend ResourceHandle operator+(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return ResourceHandle(lhs.value + rhs.value);
		}

		friend ResourceHandle operator-(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return ResourceHandle(lhs.value - rhs.value);
		}

		friend ResourceHandle operator*(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return ResourceHandle(lhs.value * rhs.value);
		}

		friend ResourceHandle operator/(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			// Check for division by zero
			if (rhs.value == 0)
			{
				// You might want to handle this case differently based on your requirements
				std::cerr << "Error: Division by zero." << std::endl;
				return ResourceHandle();
			}
			return ResourceHandle(lhs.value / rhs.value);
		}

		// Comparison operators
		friend bool operator==(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return lhs.value == rhs.value;
		}

		friend bool operator!=(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return !(lhs == rhs);
		}

		friend bool operator<(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return lhs.value < rhs.value;
		}

		friend bool operator<=(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return lhs.value <= rhs.value;
		}

		friend bool operator>(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return lhs.value > rhs.value;
		}

		friend bool operator>=(const ResourceHandle& lhs, const ResourceHandle& rhs)
		{
			return lhs.value >= rhs.value;
		}

	private:
		uint32_t value;
	};


	namespace Resource
	{
		constexpr ResourceHandle Invalid = ResourceHandle{ std::numeric_limits<uint32_t>::max() };
	}
}
