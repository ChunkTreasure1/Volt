#include "sbpch.h"
#include "FileWatcher.h"

#include <Volt/Core/Profiling.h>
#include <Volt/Core/Base.h>

#include <Volt/Utility/FileSystem.h>

FileWatcher::FileWatcher(std::chrono::duration<int32_t, std::milli> threadDelay)
	: myThreadDelay(threadDelay)
{
	myIsRunning = true;
	myWorkingDirectory = std::filesystem::current_path();
	myWatcherThread = std::thread{ &FileWatcher::Thread_QueryFileChanges, this };

}

FileWatcher::~FileWatcher()
{
	myIsRunning = false;
	myWatcherThread.join();
}

void FileWatcher::WatchFolder(const std::filesystem::path& pathToFolder)
{
	auto it = std::find(myPathsToWatch.begin(), myPathsToWatch.end(), pathToFolder);
	if (it == myPathsToWatch.end())
	{
		myPathsToWatch.emplace_back(pathToFolder);
		for (const auto& path : std::filesystem::recursive_directory_iterator(pathToFolder))
		{
			AddPathToWatch(path.path());
		}
	}
}

void FileWatcher::WatchFile(const std::filesystem::path& pathToFile)
{
	AddPathToWatch(pathToFile);
}

const FileChangedData FileWatcher::QueryChangedFile()
{
	VT_ASSERT(!myChangedFiles.empty(), "No changed files! Make sure you check first!");

	std::lock_guard<std::mutex> lock(myChangedFilesMutex);

	FileChangedData changedFile = myChangedFiles.back();
	myChangedFiles.pop();

	return changedFile;
}

void FileWatcher::AddPathToWatch(const std::filesystem::path& path)
{
	if (!myWatchedPathsTimes.contains(path))
	{
		myWatchedPathsTimes[path] = std::filesystem::last_write_time(path);
	}
}

void FileWatcher::AddChangedFile(const FileChangedData& changedFile)
{
	std::lock_guard<std::mutex> lock(myChangedFilesMutex);
	myChangedFiles.push(changedFile);
}

void FileWatcher::Thread_QueryFileChanges()
{
	VT_PROFILE_THREAD("File Watcher");

	while (myIsRunning)
	{
		VT_PROFILE_SCOPE("File Watcher");

		std::this_thread::sleep_for(myThreadDelay);

		if (myWorkingDirectory != std::filesystem::current_path() || FileSystem::globalIsOpenSaveFileOpen)
		{
			continue;
		}

		auto it = myWatchedPathsTimes.begin();
		while (it != myWatchedPathsTimes.end())
		{
			if (!std::filesystem::exists(it->first))
			{
				FileChangedData changedFile{};
				changedFile.filePath = it->first;
				changedFile.status = FileStatus::Removed;
				changedFile.lastWriteTime = it->second;

				AddChangedFile(changedFile);
				it = myWatchedPathsTimes.erase(it);
			}
			if (it != myWatchedPathsTimes.end())
			{
				it++;
			}
		}

		for (const auto& path : myPathsToWatch)
		{
			for (const auto& pathToCheckIt : std::filesystem::recursive_directory_iterator(path))
			{
				const std::filesystem::path& pathToCheck = pathToCheckIt.path();

				if (!std::filesystem::is_regular_file(pathToCheck))
				{
					continue;
				}

				const auto writeTime = std::filesystem::last_write_time(pathToCheck);
				if (!myWatchedPathsTimes.contains(pathToCheck))
				{
					myWatchedPathsTimes[pathToCheck] = writeTime;

					FileChangedData changedFile{};
					changedFile.filePath = pathToCheck;
					changedFile.status = FileStatus::Added;
					changedFile.lastWriteTime = writeTime;

					AddChangedFile(changedFile);
				}
				else if (myWatchedPathsTimes.at(pathToCheck) != writeTime)
				{
					FileChangedData changedFile{};
					changedFile.filePath = pathToCheck;
					changedFile.status = FileStatus::Modified;
					changedFile.lastWriteTime = writeTime;

					AddChangedFile(changedFile);
					myWatchedPathsTimes[pathToCheck] = writeTime;
				}
			}
		}
	}
}