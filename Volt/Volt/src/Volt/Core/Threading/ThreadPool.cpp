#include "vtpch.h"
#include "ThreadPool.h"

#include "Volt/Core/Profiling.h"

#include "Volt/Platform/ThreadUtility.h"

namespace Volt
{
	void ThreadPool::Initialize(const uint32_t threadCount)
	{
		std::call_once(m_onceFlag, [this, threadCount]()
		{
			WriteLock lock{ m_mutex };
			m_shouldStop = false;
			m_shouldCancel = false;

			m_workerThreads.reserve(threadCount);

			for (uint32_t i = 0; i < threadCount; i++)
			{
				m_workerThreads.emplace_back(std::bind(&ThreadPool::SpawnThread, this));
				SetThreadName(m_workerThreads.back().native_handle(), std::format("Worker Thread #{0}", i).c_str());
			}

			m_hasBeenInitialized = true;
			m_threadCount = threadCount;
		});
	}

	void ThreadPool::Shutdown()
	{
		if (IsRunningImpl())
		{
			WriteLock lock{ m_mutex };
			m_shouldStop = true;
		}
		else
		{
			return;
		}

		m_condition.notify_all();
		for (auto& worker : m_workerThreads)
		{
			worker.join();
		}
	}

	void ThreadPool::Cancel()
	{
		WriteLock lock{ m_mutex };
		if (IsRunningImpl())
		{
			m_shouldCancel = true;
		}
		else
		{
			return;
		}

		m_tasks.clear();
		m_condition.notify_all();
		for (auto& worker : m_workerThreads)
		{
			worker.join();
		}
	}

	void ThreadPool::SpawnThread()
	{
		VT_PROFILE_THREAD("Worker Thread");

		while (true)
		{
			bool pop = false;

			std::function<void()> task;

			{
				WriteLock lock{ m_mutex };
				m_condition.wait(lock, [this, &pop, &task]()
				{
					pop = m_tasks.try_pop(task);
					return m_shouldCancel || m_shouldStop || pop;
				});
			}

			if (m_shouldCancel || (m_shouldStop && !pop))
			{
				return;
			}

			{
				VT_PROFILE_SCOPE("Execute Task");
				task();
			}
		}
	}
}
