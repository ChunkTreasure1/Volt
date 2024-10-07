#pragma once
#include "Circuit/Widgets/Widget.h"

#include <CoreUtilities/Containers/Vector.h>

namespace Circuit
{
	enum class LayoutOrientation
	{
		Horizontal,
		Vertical,
	};
	class CIRCUIT_API LayoutWidget : public Circuit::Widget
	{
	public:
		CIRCUIT_BEGIN_ARGS(LayoutWidget)
			:_Orientation(LayoutOrientation::Vertical)
		{
		};

		CIRCUIT_ARGUMENT(LayoutOrientation, Orientation);
		CIRCUIT_END_ARGS();


		void Build(const Arguments& args);

		virtual void OnPaint(CircuitPainter& painter) override;

		void AddFixedSlice(Ref<Widget> widget, float size);

		void AddFlexibleSlice(Ref<Widget> contentWidget);

	private:

		struct Slice
		{
			Ref<Widget> widget;
			float size = -1; //for fixed size, if < 0 is flexible slice
		};
		Vector<Slice> m_slices;

		LayoutOrientation m_orientation;
	};
}
