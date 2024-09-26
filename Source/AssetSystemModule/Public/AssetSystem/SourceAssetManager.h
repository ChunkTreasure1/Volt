#pragma once

#include "AssetSystem/SourceAssetImporterRegistry.h"
#include "AssetSystem/SourceAssetImporter.h"
#include "AssetSystem/AssetManager.h"

#include <JobSystem/JobPromise.h>
#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/Containers/Queue.h>

VT_DECLARE_LOG_CATEGORY(LogSourceAssetManager, LogVerbosity::Trace);

namespace Volt
{
	class Asset;
	class VTAS_API SourceAssetManager
	{
	public:
		using ImportedCallbackFunc = std::function<void()>;

		SourceAssetManager();
		~SourceAssetManager();

		template<typename ConfigType>
		static JobFuture<Vector<Ref<Asset>>> ImportSourceAsset(const std::filesystem::path& filepath, const ConfigType& config, const SourceAssetUserImportData& userData = {})
		{
			VT_ENSURE(s_instance);

			auto importFunc = [=]() -> Vector<Ref<Asset>>
			{
				const std::string extension = filepath.extension().string();
				auto& importer = GetSourceAssetImporterRegistry().GetImporterForExtension(extension);
				return importer.Import(AssetManager::GetFilesystemPath(filepath), config, userData);
			};

			return s_instance->ImportSourceAssetInternal(std::move(importFunc), filepath.extension().string());
		}

		template<typename ConfigType>
		static void ImportSourceAsset(const std::filesystem::path& filepath, const ConfigType& config, const ImportedCallbackFunc& importedCallback, const SourceAssetUserImportData& userData = {})
		{
			VT_ENSURE(s_instance);
			VT_ENSURE(importedCallback);

			auto importFunc = [=]() -> Vector<Ref<Asset>>
			{
				const std::string extension = filepath.extension().string();
				auto& importer = GetSourceAssetImporterRegistry().GetImporterForExtension(extension);
				return importer.Import(AssetManager::GetFilesystemPath(filepath), config, userData);
			};

			s_instance->ImportSourceAssetInternal(std::move(importFunc), importedCallback, filepath.extension().string());
		}

		static SourceAssetFileInformation GetSourceAssetFileInformation(const std::filesystem::path& filepath);

	private:
		using ImportJobFunc = std::function<Vector<Ref<Asset>>()>;

		struct ImportJob
		{
			JobID jobId;
			Ref<JobPromise<Vector<Ref<Asset>>>> resultPromise;
		};

		JobFuture<Vector<Ref<Asset>>> ImportSourceAssetInternal(ImportJobFunc&& importFunc, const std::string& extension);
		void ImportSourceAssetInternal(ImportJobFunc&& importFunc, const ImportedCallbackFunc& importedCallback, const std::string& extension);

		void RunAssetImportWorker();

		inline static SourceAssetManager* s_instance = nullptr;

		std::atomic_bool m_isRunning = true;
		std::mutex m_wakeMutex;
		std::condition_variable m_wakeCondition;
		Scope<std::thread> m_assetImporterWorkerThread;

		vt::map<std::string, Scope<std::atomic_bool>> m_isImporterInUseMap;
		vt::map<std::string, Queue<ImportJob>> m_importQueue;
	};
}
