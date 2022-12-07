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
		inline static ComPtr<ID3D11DeviceContext> GetContext() { return myInstance->myContext; }
		inline static ComPtr<ID3DUserDefinedAnnotation> GetAnnotations() { return myInstance->myAnnotations; }

		static Ref<GraphicsContext> Create(GLFWwindow* aWindow);

	private:
		ComPtr<ID3D11Device> myDevice = nullptr;
		ComPtr<ID3D11DeviceContext> myContext = nullptr;
		ComPtr<ID3DUserDefinedAnnotation> myAnnotations = nullptr;

		GLFWwindow* myWindow;

		inline static GraphicsContext* myInstance = nullptr;
	};
}