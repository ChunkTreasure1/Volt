#pragma once

#include "JobSystem/Config.h"
#include "JobSystem/ThreadPool.h"

namespace Volt
{
	class VTJS_API JobSystem
	{
	public:
		JobSystem();
		~JobSystem();

		template<typename F, typename... Args>
		static auto SubmitTask(F&& f, Args&&... args)->std::future<decltype(f(args...))>;

		static uint32_t GetThreadCount();

	private:
		inline static JobSystem* s_instance = nullptr;

		ThreadPool m_threadPool;
	};

	template<typename F, typename ...Args>
	inline auto JobSystem::SubmitTask(F&& f, Args && ...args) -> std::future<decltype(f(args ...))>
	{
		VT_ENSURE(s_instance);
		return s_instance->m_threadPool.SubmitTask(f, std::forward<Args>(args)...);
	}
}
