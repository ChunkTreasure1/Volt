#pragma once

#include <Volt/Asset/Asset.h>

#include <filesystem>
#include <chrono>

struct BuildInfo
{
	std::filesystem::path buildDirectory;
	std::vector<Volt::AssetHandle> sceneHandles = {};
};

class GameBuilder
{
public:
	static void BuildGame(const BuildInfo& buildInfo);

	inline static const uint32_t GetCurrentFileNumber() { return myCurrentFileNumber; }
	inline static const bool IsBuilding() { return myIsBuilding; }
	inline static float GetCurrentBuildTime() { return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - myBuildStartTime).count(); }
	static float GetBuildProgress();
	static std::string GetCurrentFile();

private:
	static void Thread_BuildGame(const BuildInfo& buildInfo);
	static uint32_t GetRelevantFileCount(const BuildInfo& buildInfo);

	GameBuilder() = delete;

	inline static std::chrono::high_resolution_clock::time_point myBuildStartTime;
	
	inline static std::atomic_bool myIsBuilding = false;
	inline static std::atomic_uint32_t myCurrentFileNumber = 0;

	inline static std::string myCurrentFile;
	inline static std::mutex myMutex;

	inline static uint32_t myRelevantFileCount = 0;
};