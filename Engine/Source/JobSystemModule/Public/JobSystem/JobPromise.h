#pragma once

#include "JobSystem/Job.h"
#include "JobSystem/JobSystem.h"

#include <CoreUtilities/Core.h>

namespace Volt
{
	template<typename Type>
	class JobFuture
	{
	public:
		VT_INLINE const Type& Get() const
		{
			VT_ENSURE(m_associatedJob != INVALID_JOB_ID);
			JobSystem::WaitForJob(m_associatedJob);
			return *m_value;
		}

		VT_INLINE void WaitForCompletion()
		{
			VT_ENSURE(m_associatedJob != INVALID_JOB_ID);
			JobSystem::WaitForJob(m_associatedJob);
		}

	private:
		template<typename T>
		friend class JobPromise;

		JobID m_associatedJob = INVALID_JOB_ID;
		Ref<Type> m_value;
	};

	template<typename Type>
	class JobPromise
	{
	public:
		JobPromise()
		{
			m_value = CreateRef<Type>();
		}

		VT_INLINE const Type& Get() const
		{
			VT_ENSURE(m_associatedJob != INVALID_JOB_ID);
			JobSystem::WaitForJob(m_associatedJob);
			return *m_value;
		}

		VT_INLINE void WaitForCompletion()
		{
			VT_ENSURE(m_associatedJob != INVALID_JOB_ID);
			JobSystem::WaitForJob(m_associatedJob);
		}

		VT_INLINE void SetAssociatedJob(JobID id)
		{
			m_associatedJob = id;
		}

		VT_INLINE void SetValue(const Type& value)
		{
			*m_value = value;
		}

		VT_INLINE JobFuture<Type> GetFuture()
		{
			VT_ENSURE(m_associatedJob != INVALID_JOB_ID);

			JobFuture<Type> future;
			future.m_value = m_value;
			future.m_associatedJob = m_associatedJob;

			return future;
		}

	private:
		JobID m_associatedJob = INVALID_JOB_ID;
		Ref<Type> m_value;
	};

}
