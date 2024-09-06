#pragma once
#include "WindowModule/Config.h"

#include <EventSystem/Event.h>

#include <filesystem>
#include <CoreUtilities/Containers/Vector.h>

namespace Volt
{
	class WINDOWMODULE_API WindowBeginFrameEvent : public Event
	{
	public:
		WindowBeginFrameEvent()
		{
		}

		EVENT_CLASS(WindowBeginFrameEvent, "{AF873F34-6F2D-41AF-B85D-E15D6EEC5F5A}"_guid);
	};

	class WINDOWMODULE_API WindowPresentFrameEvent : public Event
	{
	public:
		WindowPresentFrameEvent()
		{
		}

		EVENT_CLASS(WindowPresentFrameEvent, "{B08F5791-DABA-4AB8-9FFB-66F0CF29DAFA}"_guid);

	};

	class WINDOWMODULE_API WindowResizeEvent : public Event
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

		std::string ToString() const override;

		EVENT_CLASS(WindowResizeEvent, "{E045B613-AE06-41C9-8759-90B7E23AEED1}"_guid);

	private:
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_x;
		uint32_t m_y;
	};

	class WINDOWMODULE_API ViewportResizeEvent : public Event
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

		std::string ToString() const override;

		EVENT_CLASS(ViewportResizeEvent, "{CF62FE90-790A-4733-B091-6E708E80EAD7}"_guid);

	private:
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_x;
		uint32_t m_y;
	};

	class WINDOWMODULE_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS(WindowCloseEvent, "{01DFAC3F-FDB8-42E3-9903-22C0F4B9736F}"_guid);
	};

	class WINDOWMODULE_API WindowTitlebarHittestEvent : public Event
	{
	public:
		WindowTitlebarHittestEvent(int x, int y, int& hit)
			: m_x(x), m_y(y), m_hit(hit)
		{
		}

		inline const int32_t GetX() const { return m_x; }
		inline const int32_t GetY() const { return m_y; }

		inline void SetHit(bool hasHit) { m_hit = (int32_t)hasHit; }

		EVENT_CLASS(WindowTitlebarHittestEvent, "{5D627292-9BA3-4E8F-9FDE-AA53393B6384}"_guid);

	private:
		int m_x;
		int m_y;
		int& m_hit;
	};

	class WINDOWMODULE_API WindowRenderEvent : public Event
	{
	public:
		WindowRenderEvent() = default;

		EVENT_CLASS(WindowRenderEvent, "{9775E1B6-2478-43FB-918D-C0B686AB328B}"_guid);
	};

	class WINDOWMODULE_API WindowDragDropEvent : public Event
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

		EVENT_CLASS(WindowDragDropEvent, "{FE76F668-D2AB-4EFF-8209-A8EFDC9EBDCF}"_guid);


	private:
		Vector<std::filesystem::path> m_paths;
	};
}
