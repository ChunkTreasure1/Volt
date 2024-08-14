#pragma once
#include "Circuit/Widgets/BuildingSyntaxUtility.h"
#include "Circuit/Config.h"
class WidgetBuilder;

namespace Circuit
{
	class CircuitPainter;
	class InputEvent;
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

		 virtual void OnInputEvent(InputEvent& inputEvent) {};

		 virtual void OnPaint(CircuitPainter& painter);

		 void RequestRebuild();

		 const std::vector<std::shared_ptr<Widget>>& GetChildren() const { return m_Children; }

		 bool IsRenderPrimitive() const;
		 RenderPrimitiveType GetRenderPrimitiveType() const;
	protected:
		template<class WidgetType>
		inline std::shared_ptr<WidgetType>& AddChildWidget(std::shared_ptr<WidgetType> Widget);

		bool m_IsRenderPrimitive = false;
		RenderPrimitiveType m_RenderPrimitiveType;
	private:
		std::vector<std::shared_ptr<Widget>> m_Children;


		float m_LocalXPosition;
		float m_LocalYPosition;
		bool m_NeedsRebuild;


	};

	template<class WidgetType>
	inline std::shared_ptr<WidgetType>& Widget::AddChildWidget(std::shared_ptr<WidgetType> Widget)
	{
		m_Children.push_back(Widget);
		return Widget;
	}
}
