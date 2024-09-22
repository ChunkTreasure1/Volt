#include "aspch.h"

#include "AssetSystem/SourceAssetImporter.h"
#include "SourceAssetManager.h"

#include <CoreUtilities/ThreadUtilities.h>

namespace Volt
{
	SourceAssetManager::SourceAssetManager()
	{
		VT_ENSURE(s_instance == nullptr);
		s_instance = this;
	
		m_assetImporterWorkerThread = CreateScope<std::thread>(std::bind(&SourceAssetManager::RunAssetImportWorker, this));
		Thread::SetThreadName(m_assetImporterWorkerThread->native_handle(), "AssetImporterWorker");
		Thread::SetThreadPriority(m_assetImporterWorkerThread->native_handle(), ThreadPriority::Low);
	}

	SourceAssetManager::~SourceAssetManager()
	{
		m_isRunning = false;
		m_wakeCondition.notify_all();

		m_assetImporterWorkerThread->join();

		s_instance = nullptr;
	}

	JobFuture<Vector<Ref<Asset>>> SourceAssetManager::ImportSourceAssetInternal(ImportJobFunc&& importFunc, const std::string& extension)
	{
		auto resultPromise = CreateRef<JobPromise<Vector<Ref<Asset>>>>();

		JobID importJobId = JobSystem::CreateJob([this, extension, importFunc, resultPromise]() 
		{
			VT_PROFILE_SCOPE("Import Asset Job");

			auto result = importFunc();

			for (const auto asset : result)
			{
				AssetManager::SaveAsset(asset);
			}

			resultPromise->SetValue(result);

			*m_isImporterInUseMap[extension] = false;
			m_wakeCondition.notify_one();
		});

		resultPromise->SetAssociatedJob(importJobId);

		ImportJob importJob;
		importJob.resultPromise = resultPromise;
		importJob.jobId = importJobId;

		m_importQueue[extension].Enqueue(std::move(importJob));
		m_wakeCondition.notify_one();

		return resultPromise->GetFuture();
	}

	void SourceAssetManager::RunAssetImportWorker()
	{
		while (m_isRunning)
		{
			for (auto& [ext, queue] : m_importQueue)
			{
				if (!m_isImporterInUseMap.contains(ext))
				{
					m_isImporterInUseMap[ext] = CreateScope<std::atomic_bool>();
					*m_isImporterInUseMap[ext] = false;
				}

				if (*m_isImporterInUseMap[ext])
				{
					continue;
				}

				ImportJob jobHolder;
				if (queue.Dequeue(jobHolder))
				{
					*m_isImporterInUseMap[ext] = true;
					JobSystem::RunJob(jobHolder.jobId);
				}
			}

			std::unique_lock lock{ m_wakeMutex };
			m_wakeCondition.wait(lock);
		}
	}
}
