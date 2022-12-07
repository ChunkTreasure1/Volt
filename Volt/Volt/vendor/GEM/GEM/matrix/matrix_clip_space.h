#pragma once
#include "type_matrix4.h"

#ifndef GEM_FORCE_RIGHT_HANDED
#define GEM_LEFT_HANDED
#endif

namespace gem
{
	// ORTHOGRAPHIC
	template<typename T>
	mat<4, 4, T> ortho(T left, T right, T bottom, T top)
	{
		mat<4, 4, T> result(static_cast<T>(1));
		result[0][0] = static_cast<T>(2) / (right - left);
		result[1][1] = static_cast<T>(2) / (top - bottom);
		result[2][2] = static_cast<T>(1);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);
		return result;
	}

	template<typename T>
	mat<4, 4, T> orthoLH(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		mat<4, 4, T> result(1);
		result[0][0] = static_cast<T>(2) / (right - left);
		result[1][1] = static_cast<T>(2) / (top - bottom);
		result[2][2] = static_cast<T>(1) / (zFar - zNear);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);
		result[3][2] = -zNear / (zFar - zNear);
		return result;
	}

	template<typename T>
	mat<4, 4, T> orthoRH(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		mat<4, 4, T> result(1);
		result[0][0] = static_cast<T>(2) / (right - left);
		result[1][1] = static_cast<T>(2) / (top - bottom);
		result[2][2] = -static_cast<T>(1) / (zFar - zNear);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);
		result[3][2] = -zNear / (zFar - zNear);
		return result;
	}

	template<typename T>
	mat<4, 4, T> ortho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
#ifdef GEM_LEFT_HANDED
		return orthoLH(left, right, bottom, top, zNear, zFar);
#else
		return orthoRH(left, right, bottom, top, zNear, zFar);
#endif
	}

	// FRUSTUM
	template<typename T>
	mat<4, 4, T> frustumLH(T left, T right, T bottom, T top, T nearVal, T farVal)
	{
		mat<4, 4, T> result(0);
		result[0][0] = (static_cast<T>(2) * nearVal) / (right - left);
		result[1][1] = (static_cast<T>(2) * nearVal) / (top - bottom);
		result[2][0] = -(right + left) / (right - left);
		result[2][1] = -(top + bottom) / (top - bottom);
		result[2][2] = farVal / (farVal - nearVal);
		result[2][3] = static_cast<T>(1);
		result[3][2] = -(farVal * nearVal) / (farVal - nearVal);
		return result;
	}

	template<typename T>
	mat<4, 4, T> frustumRH(T left, T right, T bottom, T top, T nearVal, T farVal)
	{
		mat<4, 4, T> result(0);
		result[0][0] = (static_cast<T>(2) * nearVal) / (right - left);
		result[1][1] = (static_cast<T>(2) * nearVal) / (top - bottom);
		result[2][0] = (right + left) / (right - left);
		result[2][1] = (top + bottom) / (top - bottom);
		result[2][2] = farVal / (nearVal - farVal);
		result[2][3] = static_cast<T>(-1);
		result[3][2] = -(farVal * nearVal) / (farVal - nearVal);
		return result;
	}

	template<typename T>
	mat<4, 4, T> frustum(T left, T right, T bottom, T top, T nearVal, T farVal)
	{
#ifdef GEM_LEFT_HANDED
		return frustumLH(left, right, bottom, top, nearVal, farVal);
#else
		return frustumRH(left, right, bottom, top, nearVal, farVal);
#endif
	}

	// PERSPECTIVE
	template<typename T>
	mat<4, 4, T> perspectiveRH(T fovy, T aspect, T zNear, T zFar)
	{
		assert(abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

		T const tanHalfFovy = tan(fovy / static_cast<T>(2));

		mat<4, 4, T> result(static_cast<T>(0));
		result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		result[2][2] = zFar / (zNear - zFar);
		result[2][3] = -static_cast<T>(1);
		result[3][2] = -(zFar * zNear) / (zFar - zNear);
		return result;
	}

	template<typename T>
	mat<4, 4, T> perspectiveLH(T fovy, T aspect, T zNear, T zFar)
	{
		assert(abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

		T const tanHalfFovy = tan(fovy / static_cast<T>(2));

		mat<4, 4, T> result(static_cast<T>(0));
		result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		result[2][2] = zFar / (zFar - zNear);
		result[2][3] = static_cast<T>(1);
		result[3][2] = -(zFar * zNear) / (zFar - zNear);
		return result;
	}

	template<typename T>
	mat<4, 4, T> perspective(T fovy, T aspect, T zNear, T zFar)
	{
#ifdef GEM_LEFT_HANDED
		return perspectiveLH(fovy, aspect, zNear, zFar);
#else
		return perspectiveRH(fovy, aspect, zNear, zFar);
#endif
	}
}