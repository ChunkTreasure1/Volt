#include "FileSystem.h"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Lmcons.h>

bool FileSystem::HasEnvironmentVariable(const std::string& key)
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

std::string FileSystem::GetEnvVariable(const std::string& key)
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

bool FileSystem::SetEnvVariable(const std::string& key, const std::string& value)
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

bool FileSystem::SetRegistryValue(const std::string& key, const std::string& value)
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

	rc = RegSetValueExA(hkey, "", 0, REG_SZ, (BYTE*)value.c_str(), strlen(value.c_str()) + 1);
	if (rc != ERROR_SUCCESS)
	{
		return false;
	}

	RegCloseKey(hkey);

	return true;
}

void FileSystem::StartProcess(const std::filesystem::path& processName)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.cb = sizeof(STARTUPINFO);

	CreateProcess(processName.wstring().c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

std::filesystem::path FileSystem::PickFolderDialogue()
{
	return std::filesystem::path();
}
