#include "rhipch.h"
#include "BufferView.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI 
{
	RefPtr<BufferView> BufferView::Create(const BufferViewSpecification& specification)
	{
		return RHIProxy::GetInstance().CreateBufferView(specification);
	}
}
