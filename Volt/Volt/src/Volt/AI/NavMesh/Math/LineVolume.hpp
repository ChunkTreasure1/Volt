#pragma once
#include "Line.hpp"
#include <vector>

namespace Volt
{
	template<typename T>
	class LineVolume
	{
	public:
		// Default constructor: empty LineVolume.
		LineVolume();

		// Constructor taking a list of Line that makes up the LineVolume.
		LineVolume(const std::vector<Line<T>>& aLineList);

		// Add a Line to the LineVolume.
		void AddLine(const Line<T>& aLine);

		// Returns whether a point is inside the LineVolume: it is inside when the point is
		// on the line or on the side the normal is pointing away from for all the lines in
		// the LineVolume.
		bool Inside(const gem::vec<2, T>& aPosition) const;

		std::vector<Line<T>> GetLines() const;

	private:
		std::vector<Line<T>> myLineList;
	};

	template<typename T>
	std::vector<Volt::Line<T>> Volt::LineVolume<T>::GetLines() const
	{
		return myLineList;
	}

	template<typename T>
	bool Volt::LineVolume<T>::Inside(const gem::vec<2, T>& aPosition) const
	{
		for (Line<T> line : myLineList)
		{
			if (!line.Inside(aPosition))
			{
				return false;
			}
		}
		return true;
	}

	template<typename T>
	void Volt::LineVolume<T>::AddLine(const Line<T>& aLine)
	{
		myLineList.push_back(aLine);
	}

	template<typename T>
	Volt::LineVolume<T>::LineVolume(const std::vector<Line<T>>& aLineList)
	{
		myLineList = aLineList;
	}

	template<typename T>
	Volt::LineVolume<T>::LineVolume()
	{
		myLineList.clear();
	}

}