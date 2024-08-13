#pragma once

#include "JobSystem/Config.h"

#include <CoreUtilities/Containers/ThreadSafeQueue.h>
#include <CoreUtilities/Containers/Vector.h>

#include <future>
#include <functional>

namespace Volt
{
	class VTJS_API ThreadPool
	{
	public:
		using WriteLock = std::unique_lock<std::shared_mutex>;
		using ReadLock = std::shared_lock<std::shared_mutex>;

		void Initialize(const uint32_t threadCount);
		void Shutdown();
		void Cancel();

		template<typename F, typename... Args>
		auto SubmitTask(F&& f, Args&&... args) const->std::future<decltype(f(args...))>;

		inline const bool Initialized() const { ReadLock lock{ m_mutex };  return m_hasBeenInitialized; }
		inline const bool IsRunning() const { ReadLock lock{ m_mutex };  return IsRunningImpl(); }
		inline const uint32_t GetThreadCount() const { return m_threadCount; }

	private:
		inline const bool IsRunningImpl() const
		{
			return m_hasBeenInitialized && !m_shouldStop && !m_shouldCancel;
		}

		void SpawnThread();

		Vector<std::thread> m_workerThreads;

		bool m_hasBeenInitialized = false;
		bool m_shouldStop = false;
		bool m_shouldCancel = false;

		uint32_t m_threadCount = 0;

		mutable ThreadSafeQueue<std::function<void()>> m_tasks;
		mutable std::once_flag m_onceFlag;
		mutable std::shared_mutex m_mutex;
		mutable std::condition_variable_any m_condition;
	};

	template<typename F, typename ...Args>
	inline auto ThreadPool::SubmitTask(F&& f, Args && ...args) const -> std::future<decltype(f(args ...))>
	{
		using return_t = decltype(f(args...));
		using future_t = std::future<return_t>;
		using task_t = std::packaged_task<return_t()>;

		{
			ReadLock lock{ m_mutex };
			if (m_shouldStop || m_shouldCancel)
			{
				throw std::runtime_error("Delegating task to a thread pool that has been terminated or canceled.");
			}
		}

		auto bind_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		std::shared_ptr<task_t> task = std::make_shared<task_t>(std::move(bind_func));

		future_t fut = task->get_future();
		m_tasks.emplace([task]() { (*task)(); });

		m_condition.notify_one();
		return fut;
	}
}
