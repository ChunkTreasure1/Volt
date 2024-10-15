#include "cupch.h"
#include "FileSystem.h"

#include <nfd.hpp>

#include <combaseapi.h>
#include <shlobj_core.h>
#include <Lmcons.h>

namespace FileSystem
{
	bool IsWriteable(const std::filesystem::path& path)
	{
		std::filesystem::file_status status = std::filesystem::status(path);
		return (status.permissions() & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
	}

	bool Copy(const std::filesystem::path& source, const std::filesystem::path& destination)
	{
		if (!Exists(source))
		{
			return false;
		}

		std::filesystem::copy(source, destination);
		return true;
	}

	bool CopyFileToDirectory(const std::filesystem::path& source, const std::filesystem::path& dstDir)
	{
		if (!Exists(dstDir))
		{
			return false;
		}

		std::filesystem::path newPath = dstDir / source.filename();
		Copy(source, newPath);
		return true;
	}

	bool Exists(const std::filesystem::path& path)
	{
		return std::filesystem::exists(path);
	}

	bool Remove(const std::filesystem::path& path)
	{
		return std::filesystem::remove_all(path);
	}

	bool Rename(const std::filesystem::path& filepath, const std::string& name)
	{
		if (!Exists(filepath))
		{
			return false;
		}

		const std::filesystem::path newPath = filepath.parent_path() / (name + filepath.extension().string());
		std::filesystem::rename(filepath, newPath);

		return true;
	}

	bool Move(const std::filesystem::path& filepath, const std::filesystem::path& dstDir)
	{
		if (!Exists(filepath))
		{
			return false;
		}

		const std::filesystem::path newPath = dstDir / filepath.filename();
		std::filesystem::rename(filepath, newPath);

		return true;
	}

	bool MoveDirectory(const std::filesystem::path& srcDir, const std::filesystem::path& dstDir)
	{
		if (!Exists(srcDir))
		{
			return false;
		}

		std::filesystem::rename(srcDir, dstDir);
		return true;
	}

	bool CreateDirectories(const std::filesystem::path& path)
	{
		if (Exists(path))
		{
			return true;
		}

		return std::filesystem::create_directories(path);
	}

	bool ShowDirectoryInExplorer(const std::filesystem::path& dir)
	{
		auto absolutePath = std::filesystem::canonical(dir);
		if (!std::filesystem::exists(absolutePath))
		{
			return false;
		}

		bool succeded = false;

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			PIDLIST_ABSOLUTE pidl = ILCreateFromPath(absolutePath.c_str());
			if (pidl)
			{
				hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);

				succeded = SUCCEEDED(hr);

				ILFree(pidl);
			}
		}

		CoUninitialize();
		return succeded;
	}

	bool ShowFileInExplorer(const std::filesystem::path& filepath)
	{
		auto absolutePath = std::filesystem::canonical(filepath);
		if (!Exists(absolutePath))
		{
			return false;
		}

		ShellExecute(NULL, L"explore", absolutePath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
		return true;
	}

	bool OpenFileExternally(const std::filesystem::path& filepath)
	{
		auto absolutePath = std::filesystem::canonical(filepath);
		if (!Exists(absolutePath))
		{
			return false;
		}

		ShellExecute(nullptr, L"open", absolutePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		return true;
	}

	void MoveToRecycleBin(const std::filesystem::path& path)
	{
		const auto canonicalPath = std::filesystem::canonical(path);

		if (!std::filesystem::exists(canonicalPath))
		{
			return;
		}

		std::wstring wstr = canonicalPath.wstring() + std::wstring(1, L'\0');

		SHFILEOPSTRUCT fileOp;
		fileOp.hwnd = NULL;
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = wstr.c_str();
		fileOp.pTo = NULL;
		fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT;
		SHFileOperation(&fileOp);
	}

	std::filesystem::path PickFolderDialogue(const std::filesystem::path& baseDir)
	{
		const auto absolutePath = std::filesystem::absolute(baseDir);

		NFD::UniquePath outPath;
		nfdresult_t result = NFD::PickFolder(outPath, absolutePath.string().c_str());

		switch (result)
		{
			case NFD_OKAY:
			{
				return outPath.get();
			}

			case NFD_ERROR:
			case NFD_CANCEL:
				break;
		}

		return "";
	}

	std::filesystem::path OpenFileDialogue(const Vector<FileFilter>& filters, const std::filesystem::path& baseDir)
	{
		Vector<nfdfilteritem_t> filterItems{};
		for (const auto& filter : filters)
		{
			filterItems.emplace_back(filter.name.c_str(), filter.extensions.c_str());
		}

		const auto absolutePath = std::filesystem::absolute(baseDir);

		NFD::UniquePath outPath;
		nfdresult_t result = NFD::OpenDialog(outPath, filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()), absolutePath.string().c_str());

		switch (result)
		{
			case NFD_OKAY:
			{
				return outPath.get();
			}

			case NFD_ERROR:
			case NFD_CANCEL:
				break;
		}

		return "";
	}

	std::filesystem::path SaveFileDialogue(const Vector<FileFilter>& filters, const std::filesystem::path& baseDir)
	{
		Vector<nfdfilteritem_t> filterItems{};
		for (const auto& filter : filters)
		{
			filterItems.emplace_back(filter.name.c_str(), filter.extensions.c_str());
		}

		const auto absolutePath = std::filesystem::absolute(baseDir);

		NFD::UniquePath outPath;
		nfdresult_t result = NFD::SaveDialog(outPath, filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()), absolutePath.string().c_str());

		switch (result)
		{
			case NFD_OKAY:
			{
				return outPath.get();
			}

			case NFD_ERROR:
			case NFD_CANCEL:
				break;
		}

		return "";
	}

	std::filesystem::path GetDocumentsPath()
	{
		TCHAR* path = 0;
		std::filesystem::path documentsPath;
		SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_SIMPLE_IDLIST, NULL, &path);
		documentsPath = path;
		CoTaskMemFree(path);

		return documentsPath;
	}

	std::filesystem::path GetExecutablePath()
	{
		constexpr uint64_t MAX_LENGTH = 255;
		char filename[MAX_LENGTH];
		::GetModuleFileNameA(nullptr, filename, MAX_LENGTH);

		return std::filesystem::path(filename);
	}

	bool HasEnvironmentVariable(const std::string& key)
	{
		HKEY hKey;
		LPCSTR keyPath = "Environment";
		LSTATUS lOpenStatus = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_ALL_ACCESS, &hKey);

		if (lOpenStatus == ERROR_SUCCESS)
		{
			lOpenStatus = RegQueryValueExA(hKey, key.c_str(), nullptr, nullptr, nullptr, nullptr);
			RegCloseKey(hKey);
		}

		return lOpenStatus == ERROR_SUCCESS;
	}

	bool SetEnvironmentVariable(const std::string& key, const std::string& value)
	{
		HKEY hKey;
		LPCSTR keyPath = "Environment";
		DWORD createdNewKey;
		LSTATUS lOpenStatus = RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hKey, &createdNewKey);
		if (lOpenStatus == ERROR_SUCCESS)
		{
			LSTATUS lSetStatus = RegSetValueExA(hKey, key.c_str(), 0, REG_SZ, (LPBYTE)value.c_str(), (DWORD)value.length() + 1);
			RegCloseKey(hKey);

			if (lSetStatus == ERROR_SUCCESS)
			{
				SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_BLOCK, 100, nullptr);
				return true;
			}
		}

		return false;
	}

	bool SetRegistryValue(const std::string& key, const std::string& value)
	{
		HKEY hkey;

		char valueCurrent[1000];
		DWORD type;
		DWORD size = sizeof(valueCurrent);

		int rc = RegGetValueA(HKEY_CURRENT_USER, key.c_str(), nullptr, RRF_RT_ANY, &type, valueCurrent, &size);

		bool notFound = rc == ERROR_FILE_NOT_FOUND;

		if (rc != ERROR_SUCCESS && !notFound)
		{
			// Error ?
		}

		if (!notFound)
		{
			if (type != REG_SZ)
			{
				// Error ?
			}

			if (strcmp(valueCurrent, value.c_str()) == 0)
			{
				return true;
			}
		}

		DWORD disposition;
		rc = RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, 0, 0, KEY_ALL_ACCESS, nullptr, &hkey, &disposition);
		if (rc != ERROR_SUCCESS)
		{
			return false;
		}

		rc = RegSetValueExA(hkey, "", 0, REG_SZ, (BYTE*)value.c_str(), (DWORD)strlen(value.c_str()) + 1);
		if (rc != ERROR_SUCCESS)
		{
			return false;
		}

		RegCloseKey(hkey);

		return true;
	}

	std::string GetEnvironmentVariableValue(const std::string& key)
	{
		HKEY hKey;
		LPCSTR keyPath = "Environment";
		DWORD createdNewKey;
		LSTATUS lOpenStatus = RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hKey, &createdNewKey);
		if (lOpenStatus == ERROR_SUCCESS)
		{
			DWORD valueType;
			auto* data = new char[512];
			DWORD dataSize = 512;
			LSTATUS status = RegGetValueA(hKey, nullptr, key.c_str(), RRF_RT_ANY, &valueType, (PVOID)data, &dataSize);

			RegCloseKey(hKey);

			if (status == ERROR_SUCCESS)
			{
				std::string result(data);
				delete[] data;
				return result;
			}

			delete[] data;
		}

		return std::string{};
	}

	std::string GetCurrentUserName()
	{
		std::string username;

		TCHAR name[UNLEN + 1];
		DWORD size = UNLEN + 1;

		GetUserName(name, &size);
		return std::filesystem::path(name).string();
	}

	void StartProcess(const std::filesystem::path& processPath)
	{
		std::wstring processDir = processPath.parent_path().wstring();
		std::wstring tempProcessName = processPath.wstring();
		tempProcessName.insert(tempProcessName.begin(), '\"');
		tempProcessName.push_back('\"');

		SHELLEXECUTEINFO ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = L"open";
		ShExecInfo.lpFile = tempProcessName.c_str();
		ShExecInfo.lpParameters = L"";
		ShExecInfo.lpDirectory = processDir.c_str();
		ShExecInfo.nShow = SW_SHOW;
		ShExecInfo.hInstApp = NULL;
		ShellExecuteEx(&ShExecInfo);
	}

	bool RunCommand(const std::string& aCommand)
	{
		return system(aCommand.c_str());
	}

	void Initialize()
	{
		NFD::Init();
	}

	void Shutdown()
	{
		NFD::Quit();
	}
}
