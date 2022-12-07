#pragma once

#include <gem/gem.h>

namespace Volt
{
	template<typename T>
	class Plane
	{
	public:
		// Default constructor.
		Plane();

		// Constructor taking three points where the normal is (aPoint1 - aPoint0) x (aPoint2 -aPoint0).
		Plane(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1, const gem::vec<3, T>& aPoint2);

		// Constructor taking a point and a normal.
		Plane(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aNormal);

		// Init the plane with three points, the same as the constructor above.
		void InitWith3Points(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1, const gem::vec<3, T>& aPoint2);

		// Init the plane with a point and a normal, the same as the constructor above.
		void InitWithPointAndNormal(const gem::vec<3, T>& aPoint, const gem::vec<3, T>& aNormal);

		// Returns whether a point is inside the plane: it is inside when the point is on the plane or on the side the normal is pointing away from.
		bool Inside(const gem::vec<3, T>& aPosition);

		// Returns the normal of the plane.
		const gem::vec<3, T> GetNormal();

		gem::vec<3, T> GetPosition();

	private:
		gem::vec<3, T> myPoint;
		gem::vec<3, T> myNormal;
	};

	template<typename T>
	const gem::vec<3, T> Volt::Plane<T>::GetNormal()
	{
		return myNormal;
	}

	template<typename T>
	gem::vec<3, T> Volt::Plane<T>::GetPosition()
	{
		return myPoint;
	}

	template<typename T>
	bool Volt::Plane<T>::Inside(const gem::vec<3, T>& aPosition)
	{
		T temp = aPosition.Dot(GetNormal()) - myPoint.Dot(GetNormal());
		if (temp - std::numeric_limits<T>::epsilon() * 2 <= 0)
		{
			return true;
		}
		return false;
	}

	template<typename T>
	void Volt::Plane<T>::InitWithPointAndNormal(const gem::vec<3, T>& aPoint, const gem::vec<3, T>& aNormal)
	{
		myPoint = aPoint;
		myNormal = aNormal;
	}

	template<typename T>
	void Volt::Plane<T>::InitWith3Points(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1, const gem::vec<3, T>& aPoint2)
	{
		myPoint = aPoint0;

		gem::vec<3, T> tempVect1 = aPoint1 - myPoint;
		gem::vec<3, T> tempVect2 = aPoint2 - myPoint;

		myNormal = tempVect1.Cross(tempVect2).GetNormalized();
	}

	template<typename T>
	Volt::Plane<T>::Plane(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aNormal)
	{
		myPoint = aPoint0;
		myNormal = aNormal;
	}

	template<typename T>
	Volt::Plane<T>::Plane(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1, const gem::vec<3, T>& aPoint2)
	{
		myPoint = aPoint0;

		gem::vec<3, T> tempVect1 = aPoint1 - myPoint;
		gem::vec<3, T> tempVect2 = aPoint2 - myPoint;

		myNormal = tempVect1.Cross(tempVect2).GetNormalized();
	}

	template<typename T>
	Volt::Plane<T>::Plane()
	{
		myPoint = { 0, 0, 0};
		myNormal = { 0, 0, 0 };
	}

}
