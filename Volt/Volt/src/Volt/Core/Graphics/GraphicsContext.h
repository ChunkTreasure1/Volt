#pragma once

#include "Volt/Core/Base.h"

#include <wrl.h>
#include <d3d11.h>
#include <d3d11_1.h>

struct GLFWwindow;

using namespace Microsoft::WRL;

namespace Volt
{
	class Swapchain;

	class GraphicsContext
	{
	public:
		GraphicsContext(GLFWwindow* aWindow);
		~GraphicsContext();

		void Initialize();
		void Shutdown();

		inline static GraphicsContext& Get() { return *myInstance; }
		inline static ComPtr<ID3D11Device> GetDevice() { return myInstance->myDevice; }
		inline static ComPtr<ID3D11DeviceContext> GetImmediateContext() { return myInstance->myImmediateContext; }
		inline static ComPtr<ID3D11DeviceContext> GetDeferredContext() { return myInstance->myDeferredContext; }
		inline static ComPtr<ID3DUserDefinedAnnotation> GetImmediateAnnotations() { return myInstance->myImmediateAnnotations; }
		inline static ComPtr<ID3DUserDefinedAnnotation> GetDeferredAnnotations() { return myInstance->myDeferredAnnotations; }

		static Ref<GraphicsContext> Create(GLFWwindow* aWindow);

	private:
		ComPtr<ID3D11Device> myDevice = nullptr;
		ComPtr<ID3D11DeviceContext> myImmediateContext = nullptr;
		ComPtr<ID3D11DeviceContext> myDeferredContext = nullptr;
		ComPtr<ID3DUserDefinedAnnotation> myImmediateAnnotations = nullptr;
		ComPtr<ID3DUserDefinedAnnotation> myDeferredAnnotations = nullptr;

		GLFWwindow* myWindow;

		inline static GraphicsContext* myInstance = nullptr;
	};
}