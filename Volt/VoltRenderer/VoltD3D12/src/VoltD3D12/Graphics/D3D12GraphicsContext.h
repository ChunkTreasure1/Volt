#pragma once
#include "VoltRHI/Graphics/GraphicsContext.h"

namespace Volt
{

	class D3D12GraphicsContext final : public GraphicsContext
	{
	public:
		D3D12GraphicsContext(const GraphicsContextCreateInfo& info);
		~D3D12GraphicsContext();



	protected:
		void* GetHandleImpl() override;

	private:
		void Initalize(const GraphicsContextCreateInfo& info);
		void Shutdown();


	};
}
