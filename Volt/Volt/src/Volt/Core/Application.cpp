#include "vtpch.h"
#include "Application.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Input/Input.h"
#include "Volt/Input/KeyCodes.h"

#include "Volt/Core/Window.h"
#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/SwapchainVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Core/Layer/Layer.h"
#include "Volt/Steam/SteamImplementation.h"

#include "Volt/Rendering/UIRenderer.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/DebugRenderer.h"
#include "Volt/Rendering/Buffer/UniformBufferSet.h"
#include "Volt/Rendering/Buffer/UniformBuffer.h"

#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"

#include "Volt/Core/ScopedTimer.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Project/ProjectManager.h"
#include "Volt/Project/SessionPreferences.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Discord/DiscordSDK.h"

#include "Volt/Utility/Noise.h"

#include <VoltRHI/ImGui/ImGuiImplementation.h>

///////////////// TEMPORARY //////////////////
#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/ImageView.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Buffers/VertexBuffer.h>
#include <VoltRHI/Buffers/IndexBuffer.h>

#include <VoltRHI/Descriptors/DescriptorTable.h>
//////////////////////////////////////////////

#include <Amp/AudioManager/AudioManager.h>
#include <Amp/WwiseAudioManager/WwiseAudioManager.h>
#include <Amp/WWiseEngine/WWiseEngine.h>
#include <Navigation/Core/NavigationSystem.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Volt
{
	static Ref<RHI::CommandBuffer> s_commandBuffer;
	static Ref<RHI::Shader> s_shader;
	static Ref<RHI::RenderPipeline> s_renderPipeline;

	static Ref<RHI::Image2D> s_renderTarget;
	static Ref<RHI::Image2D> s_image;

	static Ref<RHI::VertexBuffer> s_vertexBuffer;
	static Ref<RHI::IndexBuffer> s_indexBuffer;

	static Ref<RHI::DescriptorTable> s_descriptorTable;

	static Ref<Mesh> s_mesh;

	Application::Application(const ApplicationInfo& info)
		: m_frameTimer(100)
	{
		VT_CORE_ASSERT(!m_instance, "Application already exists!");
		m_instance = this;

		m_info = info;
		Log::Initialize();
		Noise::Initialize();

		if (!m_info.isRuntime)
		{
			ProjectManager::SetupWorkingDirectory();
		}

		ProjectManager::SetupProject(m_info.projectPath);
		SessionPreferences::Initialize();

		WindowProperties windowProperties{};
		windowProperties.width = info.width;
		windowProperties.height = info.height;
		windowProperties.vsync = info.useVSync;
		windowProperties.title = info.title;
		windowProperties.windowMode = info.windowMode;
		windowProperties.iconPath = info.iconPath;
		windowProperties.cursorPath = info.cursorPath;

		SetupWindowPreferences(windowProperties);

		if (m_info.isRuntime)
		{
			windowProperties.title = ProjectManager::GetProject().name;
			windowProperties.cursorPath = ProjectManager::GetProject().cursorPath;
			windowProperties.iconPath = ProjectManager::GetProject().iconPath;
		}

		m_window = Window::Create(windowProperties);
		m_window->SetEventCallback(VT_BIND_EVENT_FN(Application::OnEvent));

		FileSystem::Initialize();

		m_threadPool.Initialize(std::thread::hardware_concurrency());
		m_renderThreadPool.Initialize(std::thread::hardware_concurrency() / 2);
		m_assetmanager = CreateScope<AssetManager>();

		///// TEMPORARY /////
		{
			RHI::ShaderCompilerCreateInfo shaderCompilerInfo{};
			shaderCompilerInfo.flags = RHI::ShaderCompilerFlags::WarningsAsErrors;
			//shaderCompilerInfo.cacheDirectory = ProjectManager::GetEngineDirectory() / "Engine/Shaders/Cache";
			shaderCompilerInfo.includeDirectories =
			{
				"Engine/Shaders/Source/Includes",
				"Engine/Shaders/Source/HLSL",
				"Engine/Shaders/Source/HLSL/Includes",
				ProjectManager::GetAssetsDirectory()
			};

			m_shaderCompiler = RHI::ShaderCompiler::Create(shaderCompilerInfo);

			s_commandBuffer = RHI::CommandBuffer::Create(3, RHI::QueueType::Graphics, false);
			s_shader = RHI::Shader::Create("SimpleTriangle",
				{
					ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/Testing/SimpleMesh_vs.hlsl",
					ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/Testing/SimpleMesh_ps.hlsl"
				}, true);

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = s_shader;
			s_renderPipeline = RHI::RenderPipeline::Create(pipelineInfo);

			// Image
			{
				RHI::ImageSpecification imageSpec{};
				imageSpec.width = 512;
				imageSpec.height = 512;
				imageSpec.usage = RHI::ImageUsage::Texture;
				imageSpec.generateMips = false;

				s_image = RHI::Image2D::Create(imageSpec);
			}

			// Descriptor table
			{
				RHI::DescriptorTableSpecification descriptorTableSpec{};
				descriptorTableSpec.shader = s_shader;
				s_descriptorTable = RHI::DescriptorTable::Create(descriptorTableSpec);
				//s_descriptorTable->SetImageView(0, 0, s_image->GetView());
			}

			// Render target
			{
				RHI::ImageSpecification imageSpec{};
				imageSpec.width = 400;
				imageSpec.height = 400;
				imageSpec.usage = RHI::ImageUsage::Attachment;
				imageSpec.generateMips = false;

				s_renderTarget = RHI::Image2D::Create(imageSpec);
			}

			// Create quad buffers
			{
				// Vertex buffer
				{
					constexpr uint32_t VERTEX_COUNT = 4;

					SpriteVertex* tempVertPtr = new SpriteVertex[VERTEX_COUNT];
					tempVertPtr[0].position = { -0.5f, -0.5f, 0.f, 1.f };
					tempVertPtr[1].position = { 0.5f, -0.5f, 0.f, 1.f };
					tempVertPtr[2].position = { 0.5f,  0.5f, 0.f, 1.f };
					tempVertPtr[3].position = { -0.5f,  0.5f, 0.f, 1.f };

					tempVertPtr[0].texCoords = { 0.f, 1.f };
					tempVertPtr[1].texCoords = { 1.f, 1.f };
					tempVertPtr[2].texCoords = { 1.f, 0.f };
					tempVertPtr[3].texCoords = { 0.f, 0.f };

					tempVertPtr[0].color = { 1.f };
					tempVertPtr[1].color = { 1.f };
					tempVertPtr[2].color = { 1.f };
					tempVertPtr[3].color = { 1.f };

					s_vertexBuffer = RHI::VertexBuffer::Create(tempVertPtr, sizeof(SpriteVertex) * VERTEX_COUNT);
					delete[] tempVertPtr;
				}

				// Index Buffer
				{
					constexpr uint32_t INDEX_COUNT = 6;

					uint32_t* tempIndexPtr = new uint32_t[INDEX_COUNT];

					tempIndexPtr[0] = 0;
					tempIndexPtr[1] = 3;
					tempIndexPtr[2] = 2;

					tempIndexPtr[3] = 2;
					tempIndexPtr[4] = 1;
					tempIndexPtr[5] = 0;

					s_indexBuffer = RHI::IndexBuffer::Create(tempIndexPtr, INDEX_COUNT);
					delete[] tempIndexPtr;
				}
			}

			s_mesh = AssetManager::GetAsset<Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");
		}
		/////////////////////

		//Renderer::Initialize();
		//ShaderRegistry::Initialize();
		//Renderer::LateInitialize();

		//UIRenderer::Initialize();
		//DebugRenderer::Initialize();

		MonoScriptEngine::Initialize();

		Physics::LoadSettings();
		Physics::Initialize();
		Physics::LoadLayers();

		//Init AudioEngine
		{
			std::filesystem::path defaultPath = ProjectManager::GetAudioBanksDirectory();
			Amp::WWiseEngine::Get().InitWWise(defaultPath.c_str());
			if (FileSystem::Exists(defaultPath))
			{
				for (auto bankFile : std::filesystem::directory_iterator(ProjectManager::GetAudioBanksDirectory()))
				{
					if (bankFile.path().extension() == L".bnk")
					{
						Amp::WWiseEngine::Get().LoadBank(bankFile.path().filename().string().c_str());
					}
				}
			}
		}

		if (info.enableImGui)
		{
			RHI::ImGuiCreateInfo createInfo{};
			createInfo.swapchain = m_window->GetSwapchainPtr();
			createInfo.window = m_window->GetNativeWindow();

			m_imguiImplementation = RHI::ImGuiImplementation::Create(createInfo);
		}

		if (info.netEnabled)
		{
			m_netHandler = CreateScope<Volt::NetHandler>();
		}

		m_navigationSystem = CreateScope<Volt::AI::NavigationSystem>();

		// Extras

		if (info.enableSteam)
		{
			m_steamImplementation = SteamImplementation::Create();
		}
	}

	Application::~Application()
	{
		//GraphicsContextVolt::GetDevice()->WaitForIdle();

		m_navigationSystem = nullptr;
		m_layerStack.Clear();
		m_imguiImplementation = nullptr;
		SceneManager::Shutdown();

		m_layerStack.Clear();

		Physics::SaveLayers();
		Physics::Shutdown();
		Physics::SaveSettings();

		MonoScriptEngine::Shutdown();

		//DebugRenderer::Shutdown();
		//UIRenderer::Shutdown();

		//ShaderRegistry::Shutdown();
		//Renderer::Shutdown();

		//Amp::AudioManager::Shutdown();
		Amp::WWiseEngine::Get().TermWwise();

		m_assetmanager = nullptr;
		m_threadPool.Shutdown();
		m_renderThreadPool.Shutdown();

		//Renderer::FlushResourceQueues();

		FileSystem::Shutdown();

		m_window = nullptr;
		m_instance = nullptr;
		Log::Shutdown();
	}

	void Application::Run()
	{
		VT_PROFILE_THREAD("Main");

		m_isRunning = true;

		while (m_isRunning)
		{
			VT_PROFILE_FRAME("Frame");

			m_hasSentMouseMovedEvent = false;

			m_window->BeginFrame();

			float time = (float)glfwGetTime();
			m_currentDeltaTime = time - m_lastTotalTime;
			m_lastTotalTime = time;

			{
				VT_PROFILE_SCOPE("Application::Update");

				AppUpdateEvent updateEvent(m_currentDeltaTime * m_timeScale);
				OnEvent(updateEvent);
				Amp::WWiseEngine::Get().Update();
			}

			{
				VT_PROFILE_SCOPE("Application::Render");

				//Renderer::Flush();
				//Renderer::UpdateDescriptors();

				AppRenderEvent renderEvent;
				OnEvent(renderEvent);

				s_commandBuffer->Begin();

				RHI::Rect2D scissor = { 0, 0, 400, 400 };
				RHI::Viewport viewport{};
				viewport.width = 400.f;
				viewport.height = 400.f;
				viewport.x = 0.f;
				viewport.y = 0.f;
				viewport.minDepth = 0.f;
				viewport.maxDepth = 1.f;

				s_commandBuffer->SetViewports({ viewport });
				s_commandBuffer->SetScissors({ scissor });

				// Render target barrier
				{
					RHI::ResourceBarrierInfo barrier{};
					barrier.oldState = RHI::ResourceState::PixelShaderRead;
					barrier.newState = RHI::ResourceState::RenderTarget;
					barrier.resource = s_renderTarget;

					s_commandBuffer->ResourceBarrier({ barrier });
				}

				RHI::AttachmentInfo attInfo{};
				attInfo.view = s_renderTarget->GetView();
				attInfo.clearColor = { 0.1f, 0.1f, 0.1f, 1.f };
				attInfo.clearMode = RHI::ClearMode::Clear;

				RHI::RenderingInfo renderingInfo{};
				renderingInfo.colorAttachments = { attInfo };
				renderingInfo.renderArea = scissor;

				s_commandBuffer->BeginRendering(renderingInfo);

				s_commandBuffer->BindPipeline(s_renderPipeline);
				s_commandBuffer->BindIndexBuffer(s_mesh->GetIndexBuffer());
				s_commandBuffer->BindVertexBuffers({ s_mesh->GetVertexBuffer() }, 0);

				//s_commandBuffer->BindDescriptorTable(s_descriptorTable);

				glm::mat4 mvp = glm::perspective(glm::radians(60.f), 16.f / 9.f, 1000.f, 0.1f) * glm::lookAt(glm::vec3{ 0.f, 0.f, -200.f }, glm::vec3{ 0.f, 0.f, 0.f }, glm::vec3{ 0.f, 1.f, 0.f });
				auto constantsBuffer = s_shader->GetConstantsBuffer();
				constantsBuffer.SetMemberData("mvp", mvp);

				s_commandBuffer->PushConstants(constantsBuffer.GetBuffer(), static_cast<uint32_t>(constantsBuffer.GetSize()), 0);
				s_commandBuffer->DrawIndexed(s_mesh->GetSubMeshes().at(0).indexCount, 1, 0, 0, 0);

				s_commandBuffer->EndRendering();

				// Shader Read barrier
				{
					RHI::ResourceBarrierInfo barrier{};
					barrier.oldState = RHI::ResourceState::RenderTarget;
					barrier.newState = RHI::ResourceState::PixelShaderRead;
					barrier.resource = s_renderTarget;

					s_commandBuffer->ResourceBarrier({ barrier });
				}

				s_commandBuffer->End();
				s_commandBuffer->Execute();
			}

			if (m_info.enableImGui)
			{
				VT_PROFILE_SCOPE("Application::ImGui");

				m_imguiImplementation->Begin();

				AppImGuiUpdateEvent imguiEvent{};
				OnEvent(imguiEvent);

				if (ImGui::Begin("Test"))
				{
					ImTextureID texId = m_imguiImplementation->GetTextureID(s_renderTarget);
					ImGui::Image(texId, ImGui::GetContentRegionAvail());

					ImGui::End();
				}

				if (ImGui::Begin("Performance"))
				{
					ImGui::Text("GPU Time: %f ms", s_commandBuffer->GetExecutionTime(0));

					ImGui::End();
				}

				m_imguiImplementation->End();
			}

			if (m_info.netEnabled)
			{
				VT_PROFILE_SCOPE("Application::Net");
				m_netHandler->Update(m_currentDeltaTime);
			}

			{
				VT_PROFILE_SCOPE("Discord SDK");
				DiscordSDK::Update();
			}

			m_window->Present();

			m_frameTimer.Accumulate();
		}
	}

	void Application::OnEvent(Event& event)
	{
		VT_PROFILE_SCOPE((std::string("Application::OnEvent: ") + std::string(event.GetName())).c_str());

		if (event.GetEventType() == MouseMoved)
		{
			if (m_hasSentMouseMovedEvent)
			{
				return;
			}
			else
			{
				m_hasSentMouseMovedEvent = true;
			}
		}

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<AppUpdateEvent>(VT_BIND_EVENT_FN(Application::OnAppUpdateEvent));
		dispatcher.Dispatch<WindowCloseEvent>(VT_BIND_EVENT_FN(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>(VT_BIND_EVENT_FN(Application::OnWindowResizeEvent));
		dispatcher.Dispatch<ViewportResizeEvent>(VT_BIND_EVENT_FN(Application::OnViewportResizeEvent));
		dispatcher.Dispatch<KeyPressedEvent>(VT_BIND_EVENT_FN(Application::OnKeyPressedEvent));

		m_netHandler->OnEvent(event);

		if (m_navigationSystem)
		{
			m_navigationSystem->OnEvent(event);
		}

		for (auto layer : m_layerStack)
		{
			layer->OnEvent(event);
			if (event.handled)
			{
				break;
			}
		}
		Input::OnEvent(event);
	}

	void Application::PushLayer(Layer* layer)
	{
		m_layerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		m_layerStack.PopLayer(layer);
	}

	bool Application::OnAppUpdateEvent(AppUpdateEvent&)
	{
		if (m_steamImplementation)
		{
			m_steamImplementation->Update();
		}
		return false;
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent&)
	{
		m_isRunning = false;
		return false;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_isMinimized = true;
		}
		else
		{
			m_isMinimized = false;
		}

		m_window->Resize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool Application::OnViewportResizeEvent(ViewportResizeEvent& e)
	{
		m_window->SetViewportSize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool Application::OnKeyPressedEvent(KeyPressedEvent&)
	{
		return false;
	}

	void Application::SetupWindowPreferences(WindowProperties& windowProperties)
	{
		if (SessionPreferences::HasKey("WINDOW_WIDTH"))
		{
			windowProperties.width = static_cast<uint32_t>(SessionPreferences::GetInt("WINDOW_WIDTH"));
		}

		if (SessionPreferences::HasKey("WINDOW_HEIGHT"))
		{
			windowProperties.height = static_cast<uint32_t>(SessionPreferences::GetInt("WINDOW_HEIGHT"));
		}

		if (SessionPreferences::HasKey("WINDOW_MODE"))
		{
			windowProperties.windowMode = (WindowMode)static_cast<uint32_t>(SessionPreferences::GetInt("WINDOW_MODE"));
		}
	}
}
