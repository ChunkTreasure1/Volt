#pragma once
#include <gem/gem.h>
#include <limits>

namespace Volt
{
	template<typename T>
	class Line
	{
	public:
		// Default constructor: there is no line, the normal is the zero vector.
		Line();

		// Copy constructor.
		Line(const Line <T>& aLine);

		// Constructor that takes two points that define the line, the direction is aPoint1 - aPoint0.
		Line(const gem::vec<2, T>& aPoint0, const gem::vec<2, T>& aPoint1);
		Line(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1);

		// Init the line with two points, the same as the constructor above.
		void InitWith2Points(const gem::vec<2, T>& aPoint0, const gem::vec<2, T>& aPoint1);
		void InitWith2Points(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1);

		// Init the line with a point and a direction.
		void InitWithPointAndDirection(const gem::vec<2, T>& aPoint, const gem::vec<2, T>& aDirection);

		// Returns whether a point is inside the line: it is inside when the point is on the line or on the side the normal is pointing away from.
		bool Inside(const gem::vec<2, T>& aPosition) const;
		bool On(const gem::vec<2, T>& aPosition) const;

		// Returns whether a point is facing another line
		bool Facing(const Line& aLine) const;

		// Returns the direction of the line.
		const gem::vec<2, T> GetDirection() const;

		// Returns the normal of the line, which is (-direction.y, direction.x).
		const gem::vec<2, T> GetNormal() const;

		gem::vec<2, T> GetStart() const { return myFirstPoint; }
		gem::vec<2, T> GetEnd() const { return mySecondPoint; }
		gem::vec<2, T> GetMiddle() const { return myMiddle; }

	private:
		gem::vec<2, T> myFirstPoint;
		gem::vec<2, T> mySecondPoint;
		gem::vec<2, T> myMiddle;
	};

	template<typename T>
	void Volt::Line<T>::InitWith2Points(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1)
	{
		gem::vec2 v1 = { aPoint0.x, aPoint0.z };
		gem::vec2 v2 = { aPoint1.x, aPoint1.z };
		InitWith2Points(v1, v2);
	}

	template<typename T>
	Volt::Line<T>::Line(const gem::vec<3, T>& aPoint0, const gem::vec<3, T>& aPoint1)
	{
		gem::vec2 v1 = { aPoint0.x, aPoint0.z };
		gem::vec2 v2 = { aPoint1.x, aPoint1.z };
		InitWith2Points(v1, v2);
	}

	template<typename T>
	bool Volt::Line<T>::Facing(const Line& aLine) const
	{
		if (!Inside(aLine.myMiddle))
		{
			if (!aLine.Inside(myMiddle))
			{
				return true;
			}
		}
		return false;
	}

	template<typename T>
	const gem::vec<2, T> Volt::Line<T>::GetNormal() const
	{
		gem::vec<2, T> tempVect = { -GetDirection().y, GetDirection().x };
		return gem::normalize(tempVect);
	}

	template<typename T>
	const gem::vec<2, T> Volt::Line<T>::GetDirection() const
	{
		return { mySecondPoint.x - myFirstPoint.x, mySecondPoint.y - myFirstPoint.y };
	}

	template<typename T>
	bool Volt::Line<T>::Inside(const gem::vec<2, T>& aPosition) const
	{
		T temp = gem::dot(aPosition, GetNormal()) - gem::dot(myFirstPoint, GetNormal());
		if (temp - std::numeric_limits<T>::epsilon() * 2 <= 0)
		{
			return true;
		}
		return false;
	}

	template<typename T>
	bool Volt::Line<T>::On(const gem::vec<2, T>& aPosition) const
	{
		auto AB = (int)gem::floor(gem::distance(myFirstPoint, mySecondPoint) + 0.5f);
		auto AP = (int)gem::floor(gem::distance(aPosition, mySecondPoint) + 0.5f);
		auto PB = (int)gem::floor(gem::distance(myFirstPoint, aPosition) + 0.5f);
		if (AB == AP + PB)
		{
			return true;
		}
		return false;
	}

	template<typename T>
	void Volt::Line<T>::InitWithPointAndDirection(const gem::vec<2, T>& aPoint, const gem::vec<2, T>& aDirection)
	{
		myFirstPoint = aPoint;
		mySecondPoint = aPoint + aDirection;
		myMiddle = (myFirstPoint + mySecondPoint) / 2.f;
	}

	template<typename T>
	void Volt::Line<T>::InitWith2Points(const gem::vec<2, T>& aPoint0, const gem::vec<2, T>& aPoint1)
	{
		myFirstPoint = aPoint0;
		mySecondPoint = aPoint1;
		myMiddle = (myFirstPoint + mySecondPoint) / 2.f;
	}

	template<typename T>
	Volt::Line<T>::Line(const gem::vec<2, T>& aPoint0, const gem::vec<2, T>& aPoint1)
	{
		InitWith2Points(aPoint0, aPoint1);
	}

	template<typename T>
	Volt::Line<T>::Line(const Line <T>& aLine)
	{
		myFirstPoint = aLine.myFirstPoint;
		mySecondPoint = aLine.mySecondPoint;
		myMiddle = aLine.myMiddle;
	}

	template<typename T>
	Volt::Line<T>::Line()
	{
		myFirstPoint = { 0, 0 };
		mySecondPoint = { 0, 0 };
		myMiddle = { 0, 0 };
	}

}
