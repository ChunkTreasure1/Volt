#include "cupch.h"
#include "Math/2DShapes/Rect.h"

namespace Volt
{
	// Default constructor
	Rect::Rect()
		: m_position(0.0f, 0.0f), m_size(0.0f, 0.0f)
	{
	}

	Rect::Rect(float x, float y, float width, float height)
		: m_position(x, y), m_size(width, height)
	{
	}

	// Parameterized constructor
	Rect::Rect(const glm::vec2& position, const glm::vec2& size)
		: m_position(position), m_size(size)
	{
	}

	// Copy constructor
	Rect::Rect(const Rect& other)
		: m_position(other.m_position), m_size(other.m_size)
	{
	}

	// Getters
	glm::vec2 Rect::GetPosition() const
	{
		return m_position;
	}

	glm::vec2 Rect::GetSize() const
	{
		return m_size;
	}

	glm::vec2 Rect::GetTopLeft()
	{
		return m_position;
	}

	glm::vec2 Rect::GetBottomRight()
	{
		return m_position + m_size;
	}

	float Rect::ClampInsideX(float value)
	{
		return glm::min(m_position.x + m_size.x, glm::max(m_position.x, value));
	}

	float Rect::ClampInsideY(float value)
	{
		return glm::min(m_position.y + m_size.y, glm::max(m_position.y, value));
	}

	glm::vec2 Rect::ClampInsideRect(glm::vec2 value)
	{
		return glm::vec2(ClampInsideX(value.x), ClampInsideY(value.y));
	}

	// Setters
	void Rect::SetPosition(const glm::vec2& position)
	{
		m_position = position;
	}

	void Rect::SetSize(const glm::vec2& size)
	{
		m_size = size;
	}

	// Function to check if a point is inside the rectangle
	bool Rect::IsPointInside(const glm::vec2& point) const
	{
		// Top-left corner is (x, y)
		// Bottom-right corner is (x + width, y + height)
		return (point.x >= m_position.x &&
				point.x <= m_position.x + m_size.x &&
				point.y >= m_position.y &&
				point.y <= m_position.y + m_size.y);
	}

	// Function to check if another rectangle overlaps with this rectangle
	bool Rect::IsOverlapping(const Rect& other) const
	{
		return !(m_position.x + m_size.x < other.m_position.x ||   // this rect is left of the other
				 other.m_position.x + other.m_size.x < m_position.x ||  // this rect is right of the other
				 m_position.y + m_size.y < other.m_position.y ||   // this rect is above the other
				 other.m_position.y + other.m_size.y < m_position.y);   // this rect is below the other
	}

	void Rect::MergeRectIntoThis(const Rect& other)
	{
		//find max points before modifying this rect
		const glm::vec2 thisMax = { m_position.x + m_size.x, m_position.y + m_size.y };
		const glm::vec2 otherMax = { other.m_position.x + other.m_size.x, other.m_position.y + other.m_size.y };

		m_position.x = glm::min(m_position.x, other.m_position.x);
		m_position.y = glm::min(m_position.y, other.m_position.y);

		m_size.x = glm::max(otherMax.x, thisMax.x) - m_position.x;
		m_size.y = glm::max(otherMax.y, thisMax.y) - m_position.y;
	}

	// Destructor
	Rect::~Rect()
	{
	}

} // namespace Volt
