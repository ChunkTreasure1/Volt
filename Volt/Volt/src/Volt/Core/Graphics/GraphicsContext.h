#pragma once

#include "Volt/Core/Base.h"

#include <wrl.h>

struct GLFWwindow;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3DUserDefinedAnnotation;

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
		static ComPtr<ID3D11Device> GetDevice();
		static ComPtr<ID3D11DeviceContext> GetImmediateContext();
		static ComPtr<ID3D11DeviceContext> GetDeferredContext();
		static ComPtr<ID3DUserDefinedAnnotation> GetImmediateAnnotations();
		static ComPtr<ID3DUserDefinedAnnotation> GetDeferredAnnotations();

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