#pragma once

#include "RenderCore/Config.h"

#include <RHIModule/Core/RHIResource.h>
#include <RHIModule/Images/Image2D.h>
#include <RHIModule/Descriptors/BindlessDescriptorTable.h>
#include <RHIModule/Descriptors/ResourceHandle.h>

#include <CoreUtilities/Weak.h>
#include <CoreUtilities/Containers/FunctionQueue.h>

#include <span>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class Image2D;
		class SamplerState;
	}

	class VTRC_API BindlessResourcesManager
	{
	public:
		BindlessResourcesManager();
		~BindlessResourcesManager();

		ResourceHandle RegisterBuffer(WeakPtr<RHI::StorageBuffer> storageBuffer);
		ResourceHandle RegisterImageView(WeakPtr<RHI::ImageView> image);
		ResourceHandle RegisterSamplerState(WeakPtr<RHI::SamplerState> samplerState);

		void UnregisterBuffer(ResourceHandle handle);
		void UnregisterImageView(ResourceHandle handle, RHI::ImageViewType viewType);
		void UnregisterSamplerState(ResourceHandle handle);

		ResourceHandle GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer);

		void MarkBufferAsDirty(ResourceHandle handle);
		void MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType);
		void MarkSamplerStateAsDirty(ResourceHandle handle);

		void Update();
		void PrepareForRender();

		VT_NODISCARD VT_INLINE RefPtr<RHI::BindlessDescriptorTable> GetDescriptorTable() const { return m_bindlessDescriptorTable; }
		VT_NODISCARD VT_INLINE static BindlessResourcesManager& Get() { return *s_instance; }

	private:
		inline static BindlessResourcesManager* s_instance = nullptr;
		RefPtr<RHI::BindlessDescriptorTable> m_bindlessDescriptorTable;
	};
}
