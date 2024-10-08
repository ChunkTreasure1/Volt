#pragma once
#include "Circuit/Widgets/BuildingSyntaxUtility.h"
#include "Circuit/Config.h"

#include <CoreUtilities/Math/2DShapes/Rect.h>

namespace Circuit
{
	class CircuitPainter;
	enum class RenderPrimitiveType
	{
		Rectangle,
		Circle
	};
	class CIRCUIT_API Widget
	{
	public:
		 Widget() {};
		 virtual ~Widget() {};

	public:
		 void SetX(float x) { m_LocalXPosition = x; }
		 float GetX() const { return m_LocalXPosition; }

		 void SetY(float y) { m_LocalYPosition = y; }
		 float GetY() const { return m_LocalYPosition; }


		 void BuildBaseArgs(const CircuitBaseArgs& baseArgs);

		 virtual void OnPaint(CircuitPainter& painter);

		 virtual void CalculateBounds();
		 virtual Volt::Rect GetBounds();

		 void RequestRebuild();

		 const std::vector<Ref<Widget>>& GetChildren() const { return m_Children; }

		 bool IsRenderPrimitive() const;
		 RenderPrimitiveType GetRenderPrimitiveType() const;
	protected:
		template<class WidgetType>
		inline Ref<WidgetType>& AddChildWidget(Ref<WidgetType> Widget);

		bool m_IsRenderPrimitive = false;
		RenderPrimitiveType m_RenderPrimitiveType;


	private:
		Volt::Rect m_bounds;

		std::vector<Ref<Widget>> m_Children;


		float m_LocalXPosition = 0;
		float m_LocalYPosition = 0;
		bool m_NeedsRebuild;


	};

	template<class WidgetType>
	inline Ref<WidgetType>& Widget::AddChildWidget(Ref<WidgetType> Widget)
	{
		m_Children.push_back(Widget);
		return Widget;
	}
}
