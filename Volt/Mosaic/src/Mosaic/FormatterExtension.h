#pragma once

#include <glm/glm.hpp>

#include <format>
#include <iostream>

template<>
struct std::formatter<glm::vec2, char>
{
	bool quoted = false;

	template<class ParseContext>
	constexpr ParseContext::iterator parse(ParseContext& ctx)
	{
		return ctx.end();
	}

	template<class FmtContext>
	FmtContext::iterator format(glm::vec2 s, FmtContext& ctx) const
	{
		return std::format_to(ctx.out(), "float2({0}, {1})", s.x, s.y);
	}
};

template<>
struct std::formatter<glm::vec3, char>
{
	bool quoted = false;

	template<class ParseContext>
	constexpr ParseContext::iterator parse(ParseContext& ctx)
	{
		return ctx.end();
	}

	template<class FmtContext>
	FmtContext::iterator format(glm::vec3 s, FmtContext& ctx) const
	{
		return std::format_to(ctx.out(), "float3({0}, {1}, {2})", s.x, s.y, s.z);
	}
};

template<>
struct std::formatter<glm::vec4, char>
{
	bool quoted = false;

	template<class ParseContext>
	constexpr ParseContext::iterator parse(ParseContext& ctx)
	{
		return ctx.end();
	}

	template<class FmtContext>
	FmtContext::iterator format(glm::vec4 s, FmtContext& ctx) const
	{
		return std::format_to(ctx.out(), "float4({0}, {1}, {2}, {3})", s.x, s.y, s.z, s.w);
	}
};

template<>
struct std::formatter<glm::ivec2, char>
{
	bool quoted = false;

	template<class ParseContext>
	constexpr ParseContext::iterator parse(ParseContext& ctx)
	{
		return ctx.end();
	}

	template<class FmtContext>
	FmtContext::iterator format(glm::ivec2 s, FmtContext& ctx) const
	{
		return std::format_to(ctx.out(), "int2({0}, {1})", s.x, s.y);
	}
};

template<>
struct std::formatter<glm::ivec3, char>
{
	bool quoted = false;

	template<class ParseContext>
	constexpr ParseContext::iterator parse(ParseContext& ctx)
	{
		return ctx.end();
	}

	template<class FmtContext>
	FmtContext::iterator format(glm::ivec3 s, FmtContext& ctx) const
	{
		return std::format_to(ctx.out(), "int3({0}, {1}, {2})", s.x, s.y, s.z);
	}
};

template<>
struct std::formatter<glm::ivec4, char>
{
	bool quoted = false;

	template<class ParseContext>
	constexpr ParseContext::iterator parse(ParseContext& ctx)
	{
		return ctx.end();
	}

	template<class FmtContext>
	FmtContext::iterator format(glm::ivec4 s, FmtContext& ctx) const
	{
		return std::format_to(ctx.out(), "int4({0}, {1}, {2}, {3})", s.x, s.y, s.z, s.w);
	}
};
