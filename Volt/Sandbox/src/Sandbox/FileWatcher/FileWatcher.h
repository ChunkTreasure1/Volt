#pragma once

#include <efsw/efsw.hpp>

class FileListener;
class FileWatcher
{
public:
	FileWatcher();
	~FileWatcher();

	void AddWatch(const std::filesystem::path& path, bool recursive = true);
	void AddCallback(efsw::Actions::Action action, std::function<void(const std::filesystem::path, const std::filesystem::path)>&& callback);

	inline static FileWatcher& Get() { return *myInstance; }
private:
	inline static FileWatcher* myInstance = nullptr;

	Scope<efsw::FileWatcher> myFileWatcher;
	Scope<FileListener> myFileListener;

	std::vector<efsw::WatchID> myWatchIds;
};