#pragma once

#include "CoreUtilities/Config.h"

#include <glm/glm.hpp>

namespace Volt
{

	class VTCOREUTIL_API Rect
	{
	private:
		glm::vec2 m_position; // Top-left corner (x, y)
		glm::vec2 m_size;     // Width and Height (width, height)

	public:
		// Constructors
		Rect();
		Rect(float x, float y, float width, float height);
		Rect(const glm::vec2& position, const glm::vec2& size);

		// Copy constructor
		Rect(const Rect& other);

		// Getters
		glm::vec2 GetPosition() const;
		glm::vec2 GetSize() const;

		glm::vec2 GetTopLeft();
		glm::vec2 GetBottomRight();

		float ClampInsideX(float value);
		float ClampInsideY(float value);
		glm::vec2 ClampInsideRect(glm::vec2 value);

		// Setters
		void SetPosition(const glm::vec2& position);
		void SetSize(const glm::vec2& size);

		// Collision detection functions
		bool IsPointInside(const glm::vec2& point) const;
		bool IsOverlapping(const Rect& other) const;

		void MergeRectIntoThis(const Rect& other);

		// Destructor
		~Rect();
	};

} // namespace Volt
