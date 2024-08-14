#pragma once

#include "RenderCore/Config.h"

#include <cstdint>
#include <format>

namespace Volt
{
	class RenderGraphNullHandle;
	class VTRC_API RenderGraphResourceHandle
	{
	public:
		constexpr RenderGraphResourceHandle() : m_value(0) {}

		inline constexpr uint32_t Get() const { return m_value; }

		inline operator uint32_t() const
		{
			return m_value;
		}

		inline bool operator==(const RenderGraphResourceHandle& other)
		{
			return m_value == other.m_value;
		}

		bool operator==(const RenderGraphNullHandle& other);

	protected:
		constexpr explicit RenderGraphResourceHandle(uint32_t val) : m_value(val) {}
		constexpr explicit RenderGraphResourceHandle(size_t val) : m_value(static_cast<uint32_t>(val)) {}

		uint32_t m_value;
	};

	class VTRC_API RenderGraphNullHandle : public RenderGraphResourceHandle
	{
	public:
		RenderGraphNullHandle()
			: RenderGraphResourceHandle(std::numeric_limits<uint32_t>::max())
		{ }
	};

	class VTRC_API RenderGraphBufferHandle : public RenderGraphResourceHandle
	{
	public:
		RenderGraphBufferHandle() = default;
		RenderGraphBufferHandle(const RenderGraphNullHandle& other)
		{
			m_value = other.Get();
		}

		RenderGraphBufferHandle& operator=(const RenderGraphNullHandle& rhs)
		{
			m_value = rhs.Get();
			return *this;
		}
	};

	class VTRC_API RenderGraphUniformBufferHandle : public RenderGraphResourceHandle
	{
	public:
		RenderGraphUniformBufferHandle() = default;
		RenderGraphUniformBufferHandle(const RenderGraphNullHandle& other)
		{
			m_value = other.Get();
		}

		RenderGraphUniformBufferHandle& operator=(const RenderGraphNullHandle& rhs)
		{
			m_value = rhs.Get();
			return *this;
		}
	};

	class VTRC_API RenderGraphImageHandle : public RenderGraphResourceHandle
	{
	public:
		RenderGraphImageHandle() = default;
		RenderGraphImageHandle(const RenderGraphNullHandle& other)
		{
			m_value = other.Get();
		}

		RenderGraphImageHandle& operator=(const RenderGraphNullHandle& rhs)
		{
			m_value = rhs.Get();
			return *this;
		}
	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<Volt::RenderGraphResourceHandle>
	{
		std::size_t operator()(const Volt::RenderGraphResourceHandle& handle) const
		{
			return std::hash<uint32_t>()(handle.Get());
		}
	};


	template<>
	struct hash<Volt::RenderGraphBufferHandle>
	{
		std::size_t operator()(const Volt::RenderGraphBufferHandle& handle) const
		{
			return std::hash<uint32_t>()(handle.Get());
		}
	};

	template<>
	struct hash<Volt::RenderGraphUniformBufferHandle>
	{
		std::size_t operator()(const Volt::RenderGraphUniformBufferHandle& handle) const
		{
			return std::hash<uint32_t>()(handle.Get());
		}
	};

	template<>
	struct hash<Volt::RenderGraphImageHandle>
	{
		std::size_t operator()(const Volt::RenderGraphImageHandle& handle) const
		{
			return std::hash<uint32_t>()(handle.Get());
		}
	};

	template <>
	struct formatter<Volt::RenderGraphResourceHandle> : formatter<string>
	{
		auto format(Volt::RenderGraphResourceHandle id, format_context& ctx) const
		{
			return formatter<string>::format(
			  std::format("{}", id.Get()), ctx);
		}
	};

	template <>
	struct formatter<Volt::RenderGraphBufferHandle> : formatter<string>
	{
		auto format(Volt::RenderGraphBufferHandle id, format_context& ctx) const
		{
			return formatter<string>::format(
			  std::format("{}", id.Get()), ctx);
		}
	};

	template <>
	struct formatter<Volt::RenderGraphUniformBufferHandle> : formatter<string>
	{
		auto format(Volt::RenderGraphUniformBufferHandle id, format_context& ctx) const
		{
			return formatter<string>::format(
			  std::format("{}", id.Get()), ctx);
		}
	};
}
