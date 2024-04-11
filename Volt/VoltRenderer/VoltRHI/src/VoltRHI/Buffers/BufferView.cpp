#include "rhipch.h"
#include "BufferView.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI 
{
	Ref<BufferView> BufferView::Create(const BufferViewSpecification& specification)
	{
		return RHIProxy::GetInstance().CreateBufferView(specification);
	}
}
