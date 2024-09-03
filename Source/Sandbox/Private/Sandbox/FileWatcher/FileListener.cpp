#include "sbpch.h"
#include "FileListener.h"

void FileListener::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename)
{
	for (const auto& callback : myCallbacks[action])
	{
		const std::filesystem::path newPath = std::filesystem::path(dir) / std::filesystem::path(filename);
		const std::filesystem::path oldPath = oldFilename;

		callback(newPath, oldPath);
	}
}

void FileListener::AddCallback(std::function<void(const std::filesystem::path, const std::filesystem::path)>&& callback, efsw::Actions::Action action)
{
	myCallbacks[action].emplace_back(std::move(callback));
}