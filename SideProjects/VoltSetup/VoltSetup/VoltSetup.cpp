// VoltSetup.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <filesystem>

inline bool HasEnvironmentVariable(const std::string& key)
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

inline bool SetEnvVariable(const std::string& key, const std::string& value)
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

inline bool SetRegistryValue(const std::string& key, const std::string& value)
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

inline std::string GetEnvVariable(const std::string& key)
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
	}

	return std::string{};
}

int main()
{
	if (HasEnvironmentVariable("VOLT_PATH"))
	{
		if (GetEnvVariable("VOLT_PATH") != std::filesystem::current_path().parent_path().string())
		{
			SetEnvVariable("VOLT_PATH", std::filesystem::current_path().parent_path().string());
		}
	}
	else
	{
		SetEnvVariable("VOLT_PATH", std::filesystem::current_path().parent_path().string());
	}

	const std::string sandboxLaunchCommand = std::filesystem::current_path().parent_path().string() + "\\Sandbox.exe %1";

	SetRegistryValue(R"(Software\Classes\.vtproj)", "Volt.Sandbox");
	SetRegistryValue(R"(Software\Classes\.vtproj\Content Type)", "text/plain");
	SetRegistryValue(R"(Software\Classes\.vtproj\PerceivedType)", "text");
	SetRegistryValue(R"(Software\Classes\Volt.Sandbox)", "Volt Sandbox");
	SetRegistryValue(R"(Software\Classes\Volt.Sandbox\Shell\Open\Command)", sandboxLaunchCommand);

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
