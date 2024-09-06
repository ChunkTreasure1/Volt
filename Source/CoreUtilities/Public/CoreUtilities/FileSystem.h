#pragma once

#include "CoreUtilities/Config.h"
#include "CoreUtilities/Containers/Vector.h"

#include <filesystem>

struct FileFilter
{
	std::string name;
	std::string extensions;
};

namespace FileSystem
{
	extern VTCOREUTIL_API bool IsWriteable(const std::filesystem::path& path);
	extern VTCOREUTIL_API bool Copy(const std::filesystem::path& source, const std::filesystem::path& destination);
	extern VTCOREUTIL_API bool CopyFileToDirectory(const std::filesystem::path& source, const std::filesystem::path& dstDir);
	extern VTCOREUTIL_API bool Exists(const std::filesystem::path& path);
	extern VTCOREUTIL_API bool Remove(const std::filesystem::path& path);
	extern VTCOREUTIL_API bool Rename(const std::filesystem::path& filepath, const std::string& name);
	extern VTCOREUTIL_API bool Move(const std::filesystem::path& filepath, const std::filesystem::path& dstDir);
	extern VTCOREUTIL_API bool MoveDirectory(const std::filesystem::path& srcDir, const std::filesystem::path& dstDir);
	extern VTCOREUTIL_API bool CreateDirectories(const std::filesystem::path& path);
	
	extern VTCOREUTIL_API bool ShowDirectoryInExplorer(const std::filesystem::path& dir);
	extern VTCOREUTIL_API bool ShowFileInExplorer(const std::filesystem::path& filepath);
	extern VTCOREUTIL_API bool OpenFileExternally(const std::filesystem::path& filepath);
	extern VTCOREUTIL_API void MoveToRecycleBin(const std::filesystem::path& path);

	extern VTCOREUTIL_API std::filesystem::path PickFolderDialogue(const std::filesystem::path& baseDir);
	extern VTCOREUTIL_API std::filesystem::path OpenFileDialogue(const Vector<FileFilter>& filters, const std::filesystem::path& baseDir);
	extern VTCOREUTIL_API std::filesystem::path SaveFileDialogue(const Vector<FileFilter>& filters, const std::filesystem::path& baseDir);
	extern VTCOREUTIL_API std::filesystem::path GetDocumentsPath();
	extern VTCOREUTIL_API std::filesystem::path GetExecutablePath();

	extern VTCOREUTIL_API void Initialize();
	extern VTCOREUTIL_API void Shutdown();
}
