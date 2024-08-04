#pragma once

#include "Event.h"
#include "Volt/Asset/Asset.h"

#include <CoreUtilities/Containers/Vector.h>

#include <sstream>
#include <filesystem>

namespace Volt
{
	class AppBeginFrameEvent : public Event
	{
	public:
		AppBeginFrameEvent()
		{ }

		EVENT_CLASS_TYPE(BeginFrame);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class AppPresentFrameEvent : public Event
	{
	public:
		AppPresentFrameEvent()
		{ }

		EVENT_CLASS_TYPE(PresentFrame);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
			: m_width(width), m_height(height), m_x(x), m_y(y)
		{
		}

		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_width(width), m_height(height), m_x(0), m_y(0)
		{
		}

		//Getting
		inline const uint32_t GetWidth() const { return m_width; }
		inline const uint32_t GetHeight() const { return m_height; }
		inline const uint32_t GetX() const { return m_x; }
		inline const uint32_t GetY() const { return m_y; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_width << ", " << m_height << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_x;
		uint32_t m_y;
	};

	class ViewportResizeEvent : public Event
	{
	public:
		ViewportResizeEvent(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
			: m_width(width), m_height(height), m_x(x), m_y(y)
		{
		}
		//Getting
		inline const uint32_t GetWidth() const { return m_width; }
		inline const uint32_t GetHeight() const { return m_height; }
		inline const uint32_t GetX() const { return m_x; }
		inline const uint32_t GetY() const { return m_y; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ViewportResizeEvent: " << m_width << ", " << m_height << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(ViewportResize);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_x;
		uint32_t m_y;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class WindowTitlebarHittestEvent : public Event
	{
	public:
		WindowTitlebarHittestEvent(int x, int y, int& hit)
			: m_x(x), m_y(y), m_hit(hit)
		{
		}

		inline const int32_t GetX() const { return m_x; }
		inline const int32_t GetY() const { return m_y; }

		inline void SetHit(bool hasHit) { m_hit = (int32_t)hasHit; }

		EVENT_CLASS_TYPE(WindowTitlebarHittest);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		int m_x;
		int m_y;
		int& m_hit;
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent(float timestep)
			: m_timestep(timestep)
		{
		}

		inline const float& GetTimestep() { return m_timestep; }

		EVENT_CLASS_TYPE(AppUpdate);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		float m_timestep;
	};

	class AppPostFrameUpdateEvent : public Event
	{
	public:
		AppPostFrameUpdateEvent(float timestep)
			: m_timestep(timestep)
		{ }

		inline const float& GetTimestep() { return m_timestep; }

		EVENT_CLASS_TYPE(AppPostFrameUpdate);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		float m_timestep;
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class AppLogEvent : public Event
	{
	public:
		AppLogEvent(const std::string& message, const std::string& severity)
			: m_message(message), m_severity(severity)
		{
		}

		inline const std::string& GetMessage() { return m_message; }
		inline const std::string& GetSeverity() { return m_severity; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AppLogEvent: " << m_message << ", " << m_severity << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(AppLog);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		std::string m_message;
		std::string m_severity;
	};

	class AppImGuiUpdateEvent : public Event
	{
	public:
		AppImGuiUpdateEvent() = default;

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AppImGuiUpdateEvent" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(AppImGuiUpdate);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class WindowDragDropEvent : public Event
	{
	public:
		WindowDragDropEvent(int32_t count, const char** paths)
		{
			for (int32_t i = 0; i < count; ++i)
			{
				m_paths.push_back(paths[i]);
			}
		}

		inline const Vector <std::filesystem::path>& GetPaths() const { return m_paths; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EditorDragDrop Accepted!" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowDragDrop);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		Vector<std::filesystem::path> m_paths;
	};

	class OnScenePlayEvent : public Event
	{
	public:
		OnScenePlayEvent() = default;

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnScenePlay" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(OnScenePlay);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class OnSceneStopEvent : public Event
	{
	public:
		OnSceneStopEvent() = default;

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnSceneStop" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(OnSceneStop);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class Scene;
	class OnSceneLoadedEvent : public Event
	{
	public:
		OnSceneLoadedEvent(Ref<Volt::Scene> aScene)
			: myScene(aScene)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnSceneLoaded" << std::endl;
			return ss.str();
		}

		inline Ref<Volt::Scene> GetScene() const { return myScene; }

		EVENT_CLASS_TYPE(OnSceneLoaded);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		Ref<Volt::Scene> myScene;
	};

	class OnSceneTransitionEvent : public Event
	{
	public:
		OnSceneTransitionEvent(Volt::AssetHandle aHandle)
			: myHandle(aHandle)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnSceneTransition" << std::endl;
			return ss.str();
		}

		inline Volt::AssetHandle GetHandle() const { return myHandle; }

		EVENT_CLASS_TYPE(OnSceneTransition);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		Volt::AssetHandle myHandle;
	};
}
