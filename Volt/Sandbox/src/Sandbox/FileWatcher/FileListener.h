#pragma once

#include <CoreUtilities/Containers/Vector.h>

#include <efsw/efsw.hpp>

class FileListener : public efsw::FileWatchListener
{
public:
	void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename /* = "" */) override;
	void AddCallback(std::function<void(const std::filesystem::path, const std::filesystem::path)>&& callback, efsw::Actions::Action action);

private:
	std::unordered_map<efsw::Actions::Action, Vector<std::function<void(const std::filesystem::path, const std::filesystem::path)>>> myCallbacks;
};
