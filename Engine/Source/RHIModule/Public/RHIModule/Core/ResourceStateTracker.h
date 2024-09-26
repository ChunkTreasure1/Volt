#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHIResource.h"
#include "RHIModule/Core/RHICommon.h"
#include <RHIModule/Core/ResourceStateTracker.h>

#include <CoreUtilities/Containers/Map.h>

#include <mutex>

namespace Volt::RHI
{
	struct ResourceState
	{
		BarrierStage stage;
		BarrierAccess access;
		ImageLayout layout;
	};

	class VTRHI_API ResourceStateTracker : public RHIInterface
	{
	public:
		ResourceStateTracker() = default;
		~ResourceStateTracker() override = default;

		void AddResource(WeakPtr<RHIResource> resource, BarrierStage initialStage, BarrierAccess initialAccess, ImageLayout initialLayout = ImageLayout::Undefined);
		void RemoveResource(WeakPtr<RHIResource> resource);

		void TransitionResource(WeakPtr<RHIResource> resource, BarrierStage dstStage, BarrierAccess dstAccess, ImageLayout dstLayout = ImageLayout::Undefined);
		const ResourceState& GetCurrentResourceState(WeakPtr<RHIResource> resource);

	protected:
		void* GetHandleImpl() const override;

	private:
		std::mutex m_mutex;
		vt::map<WeakPtr<RHIResource>, ResourceState> m_resourceStates;
	};
}
