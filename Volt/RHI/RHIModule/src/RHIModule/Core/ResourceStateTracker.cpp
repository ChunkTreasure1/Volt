#include "rhipch.h"
#include "ResourceStateTracker.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	void ResourceStateTracker::AddResource(WeakPtr<RHIResource> resource, BarrierStage initialStage, BarrierAccess initialAccess, ImageLayout initialLayout)
	{
		std::scoped_lock lock{ m_mutex };
		VT_ENSURE(!m_resourceStates.contains(resource));

		ResourceState state{};
		state.stage = initialStage;
		state.access = initialAccess;
		state.layout = initialLayout;

		m_resourceStates[resource] = state;
	}
	
	void ResourceStateTracker::RemoveResource(WeakPtr<RHIResource> resource)
	{
		std::scoped_lock lock{ m_mutex };
		VT_ENSURE(m_resourceStates.contains(resource));
		m_resourceStates.erase(resource);
	}
	
	void ResourceStateTracker::TransitionResource(WeakPtr<RHIResource> resource, BarrierStage dstStage, BarrierAccess dstAccess, ImageLayout dstLayout)
	{
		std::scoped_lock lock{ m_mutex };
		VT_ENSURE(m_resourceStates.contains(resource));

		auto& state = m_resourceStates.at(resource);
		state.stage = dstStage;
		state.access = dstAccess;
		state.layout = dstLayout;
	}
	
	const ResourceState& ResourceStateTracker::GetCurrentResourceState(WeakPtr<RHIResource> resource)
	{
		std::scoped_lock lock{ m_mutex };
		VT_ENSURE(m_resourceStates.contains(resource));

		return m_resourceStates.at(resource);
	}
	
	void* ResourceStateTracker::GetHandleImpl() const
	{
		return nullptr;
	}
}
