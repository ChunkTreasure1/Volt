#include "vtpch.h"
#include "GraphicsContext.h"

#include "Volt/Core/Graphics/Swapchain.h"
#include "Volt/Log/Log.h"

#include "Volt/Utility/DirectXUtils.h"

namespace Volt
{
	GraphicsContext::GraphicsContext(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		VT_CORE_ASSERT(!myInstance, "Context already exists!");
		myInstance = this;

		Initialize();
	}

	GraphicsContext::~GraphicsContext()
	{
		Shutdown();
		myInstance = nullptr;
	}

	void GraphicsContext::Initialize()
	{
		uint32_t createDeviceFlags = 0;

#ifdef VT_DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL maxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		VT_DX_CHECK(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, myDevice.GetAddressOf(), &maxSupportedFeatureLevel, myContext.GetAddressOf()));
		myContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)myAnnotations.GetAddressOf());
	}

	void GraphicsContext::Shutdown()
	{
#ifdef VT_DEBUG
		ID3D11Debug* debug = nullptr;
		myDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));

		debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		debug->Release();
#endif
		myContext = nullptr;
		myDevice = nullptr;

	}

	Ref<GraphicsContext> GraphicsContext::Create(GLFWwindow* aWindow)
	{
		return CreateRef<GraphicsContext>(aWindow);
	}

}