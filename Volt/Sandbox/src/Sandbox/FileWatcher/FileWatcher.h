#pragma once

#include <mutex>
#include <thread>
#include <filesystem>
#include <queue>

enum class FileStatus
{
	Added,
	Removed,
	Modified,
	Moved
};

struct FileChangedData
{
	FileStatus status;
	std::filesystem::path filePath;
	std::filesystem::file_time_type lastWriteTime;
};

class FileWatcher
{
public:
	FileWatcher(std::chrono::duration<int32_t, std::milli> threadDelay);
	~FileWatcher();

	void WatchFolder(const std::filesystem::path& pathToFolder);
	void WatchFile(const std::filesystem::path& pathToFile);

	inline const bool AnyFileChanged() const { return !myChangedFiles.empty(); }
	const FileChangedData QueryChangedFile();

private:
	void AddPathToWatch(const std::filesystem::path& path);
	void AddChangedFile(const FileChangedData& changedFile);
	
	void Thread_QueryFileChanges();

	std::queue<FileChangedData> myChangedFiles;
	std::chrono::duration<int32_t, std::milli> myThreadDelay;

	std::vector<std::filesystem::path> myPathsToWatch;
	std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> myWatchedPathsTimes;

	std::thread myWatcherThread;
	std::mutex myChangedFilesMutex;
	std::atomic_bool myIsRunning = false;

	std::filesystem::path myWorkingDirectory;
};