#pragma once
#include "Circuit/Widgets/BuildingSyntaxUtility.h"
#include "Circuit/CircuitCoreDefines.h"
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
	class Widget
	{
	public:
		CIRCUIT_API Widget() {};
		CIRCUIT_API virtual ~Widget() {};

	public:
		CIRCUIT_API void SetX(float x) { m_LocalXPosition = x; }
		CIRCUIT_API float GetX() const { return m_LocalXPosition; }

		CIRCUIT_API void SetY(float y) { m_LocalYPosition = y; }
		CIRCUIT_API float GetY() const { return m_LocalYPosition; }


		CIRCUIT_API void BuildBaseArgs(const CircuitBaseArgs& baseArgs);

		CIRCUIT_API virtual void OnInputEvent(InputEvent& inputEvent) {};

		CIRCUIT_API virtual void OnPaint(CircuitPainter& painter);

		CIRCUIT_API void RequestRebuild();

		CIRCUIT_API const std::vector<std::shared_ptr<Widget>>& GetChildren() const { return m_Children; }

		CIRCUIT_API bool IsRenderPrimitive() const;
		CIRCUIT_API RenderPrimitiveType GetRenderPrimitiveType() const;
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
