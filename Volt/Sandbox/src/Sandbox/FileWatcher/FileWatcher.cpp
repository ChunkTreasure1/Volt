#include "sbpch.h"
#include "FileWatcher.h"

#include "Sandbox/FileWatcher/FileListener.h"

FileWatcher::FileWatcher()
{
	VT_CORE_ASSERT(!myInstance, "Instance already exists!");
	myInstance = this;

	myFileWatcher = CreateScope<efsw::FileWatcher>();
	myFileListener = CreateScope<FileListener>();

	myFileWatcher->watch();
}

FileWatcher::~FileWatcher()
{
	for (const auto& id : myWatchIds)
	{
		myFileWatcher->removeWatch(id);
	}

	myWatchIds.clear();
	myFileWatcher = nullptr;
	myFileListener = nullptr;

	myInstance = nullptr;
}

void FileWatcher::AddWatch(const std::filesystem::path& path, bool recursive) 
{
	efsw::WatchID watchId = myFileWatcher->addWatch(path.string(), myFileListener.get());
	myWatchIds.emplace_back(watchId);
}

void FileWatcher::AddCallback(efsw::Actions::Action action, std::function<void(const std::filesystem::path, const std::filesystem::path)>&& callback)
{
	myFileListener->AddCallback(std::move(callback), action);
}
