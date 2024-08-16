#include "vtpch.h"
#include "FileSystem.h"

#include "Volt/Core/Application.h"
#include "Volt/Project/ProjectManager.h"

#include <CoreUtilities/Platform/Windows/VoltWindows.h>

#include <nfd.hpp>

#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Lmcons.h>

namespace FileSystem
{
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

	bool SetEnvVariable(const std::string& key, const std::string& value)
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

	std::string GetEnvVariable(const std::string& key)
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
}
