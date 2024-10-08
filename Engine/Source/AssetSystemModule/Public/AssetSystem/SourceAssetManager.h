#pragma once

#include "AssetSystem/Config.h"

#include "AssetSystem/SourceAssetImporterRegistry.h"
#include "AssetSystem/SourceAssetImporter.h"
#include "AssetSystem/AssetManager.h"

#include <JobSystem/JobPromise.h>
#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/Containers/ThreadSafeQueue.h>

VT_DECLARE_LOG_CATEGORY_EXPORT(VTAS_API, LogSourceAssetManager, LogVerbosity::Trace);

namespace Volt
{
	class Asset;
	class VTAS_API SourceAssetManager
	{
	public:
		using ImportedCallbackFunc = std::function<void(Vector<Ref<Asset>>)>;

		SourceAssetManager();
		~SourceAssetManager();

		template<typename ConfigType>
		static JobFuture<Vector<Ref<Asset>>> ImportSourceAsset(const std::filesystem::path& filepath, const ConfigType& config, const SourceAssetUserImportData& userData = {})
		{
			VT_ENSURE(s_instance);
			VT_ENSURE(!filepath.empty());

			auto importFunc = [=]() -> Vector<Ref<Asset>>
			{
				const std::string extension = filepath.extension().string();
				auto& importer = GetSourceAssetImporterRegistry().GetImporterForExtension(extension);
				return importer.Import(AssetManager::GetFilesystemPath(filepath), config, userData);
			};

			return s_instance->ImportSourceAssetInternal(std::move(importFunc), filepath);
		}

		template<typename ConfigType>
		static void ImportSourceAsset(const std::filesystem::path& filepath, const ConfigType& config, const ImportedCallbackFunc& importedCallback, const SourceAssetUserImportData& userData = {})
		{
			VT_ENSURE(s_instance);
			VT_ENSURE(importedCallback);
			VT_ENSURE(!filepath.empty());

			auto importFunc = [=]() -> Vector<Ref<Asset>>
			{
				const std::string extension = filepath.extension().string();
				auto& importer = GetSourceAssetImporterRegistry().GetImporterForExtension(extension);
				return importer.Import(AssetManager::GetFilesystemPath(filepath), config, userData);
			};

			s_instance->ImportSourceAssetInternal(std::move(importFunc), importedCallback, filepath);
		}

		static SourceAssetFileInformation GetSourceAssetFileInformation(const std::filesystem::path& filepath);

	private:
		using ImportJobFunc = std::function<Vector<Ref<Asset>>()>;

		struct ImportJob
		{
			JobID jobId;
			Ref<JobPromise<Vector<Ref<Asset>>>> resultPromise;
		
			std::string debugString;
		};

		JobFuture<Vector<Ref<Asset>>> ImportSourceAssetInternal(ImportJobFunc&& importFunc, const std::filesystem::path& filepath);
		void ImportSourceAssetInternal(ImportJobFunc&& importFunc, const ImportedCallbackFunc& importedCallback, const std::filesystem::path& filepath);

		ThreadSafeQueue<ImportJob>& GetOrCreateQueue(const std::string& extension);
		void RunAssetImportWorker();

		inline static SourceAssetManager* s_instance = nullptr;

		std::atomic_bool m_isRunning = true;
		std::mutex m_wakeMutex;
		std::condition_variable m_wakeCondition;
		Scope<std::thread> m_assetImporterWorkerThread;

		vt::map<std::string, Scope<std::atomic_bool>> m_isImporterInUseMap;
		vt::map<std::string, Scope<ThreadSafeQueue<ImportJob>>> m_importQueues;
	};
}
