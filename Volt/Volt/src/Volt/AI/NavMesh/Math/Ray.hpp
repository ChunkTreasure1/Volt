#pragma once

#include <gem/gem.h>

namespace Volt 
{
	template<class T>
	class Ray
	{
	public:
		// Default constructor: there is no ray, the origin and direction are the
		// zero vector.
		Ray();

		// Copy constructor.
		Ray(const Ray<T>& aRay);

		// Constructor that takes two points that define the ray, the direction is
		// aPoint - aOrigin and the origin is aOrigin.
		Ray(const gem::vec<3, T>& aOrigin, const gem::vec<3, T>& aPoint);

		// Init the ray with two points, the same as the constructor above.
		void InitWith2Points(const gem::vec<3, T>& aOrigin, const gem::vec<3, T>& aPoint);

		// Init the ray with an origin and a direction.
		void InitWithOriginAndDirection(const gem::vec<3, T>& aOrigin, const gem::vec<3, T>&
			aDirection);

		gem::vec<3, T> GetOrigin() { return myOrigin; };
		gem::vec<3, T> GetDirection() { return myDirection; };

	private:
		gem::vec<3, T> myOrigin;
		gem::vec<3, T> myDirection;
	};

	template<class T>
	void Volt::Ray<T>::InitWithOriginAndDirection(const gem::vec<3, T>& aOrigin, const gem::vec<3, T>& aDirection)
	{
		myOrigin = aOrigin;
		myDirection = aDirection;
	}

	template<class T>
	void Volt::Ray<T>::InitWith2Points(const gem::vec<3, T>& aOrigin, const gem::vec<3, T>& aPoint)
	{
		myOrigin = aOrigin;
		myDirection = aPoint;
	}

	template<class T>
	Volt::Ray<T>::Ray(const gem::vec<3, T>& aOrigin, const gem::vec<3, T>& aPoint)
	{
		myOrigin = aOrigin;
		myDirection = aPoint;
	}

	template<class T>
	Volt::Ray<T>::Ray(const Ray<T>& aRay)
	{
		myOrigin = aRay.myOrigin;
		myDirection = aRay.myDirection;
	}

	template<class T>
	Volt::Ray<T>::Ray()
	{
		myOrigin = {0, 0, 0};
		myDirection = { 0, 0, 0 };
	}

}