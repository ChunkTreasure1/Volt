#include "vtpch.h"
#include "SessionPreferences.h"

#include "Volt/Core/Application.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Utility/StringUtility.h"

namespace Volt
{
	void SessionPreferences::Initialize()
	{
		myRegistry.clear();

		const auto regPath = GetRegistryPath();

		HKEY regKey{};
		DWORD createdNewKey{};
		DWORD openStatus = RegCreateKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &regKey, &createdNewKey);
		if (openStatus != ERROR_SUCCESS)
		{
			return;
		}

		DWORD valueCount{};
		DWORD maxValueNameLen{};
		LONG retCode = RegQueryInfoKey(regKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &valueCount, &maxValueNameLen, nullptr, nullptr, nullptr);

		// Add NUL char
		maxValueNameLen++;

		auto nameBuffer = std::make_unique<wchar_t[]>(maxValueNameLen);
		
		for (DWORD index = 0; index < valueCount; index++)
		{
			DWORD valueNameLen = maxValueNameLen;
			DWORD valueType{};

			retCode = RegEnumValue(regKey, index, nameBuffer.get(), &valueNameLen, nullptr, &valueType, nullptr, nullptr);

			std::wstring wKey{ nameBuffer.get(), valueNameLen };

			if (valueType == REG_SZ)
			{
				myRegistry[Utility::ToString(wKey)] = std::string{};
			}
			else if (valueType == REG_DWORD)
			{
				myRegistry[Utility::ToString(wKey)] = int32_t{0};
			}
			else if (valueType == REG_BINARY)
			{
				myRegistry[Utility::ToString(wKey)] = float(0.f);
			}
		}

		for (auto& [key, value] : myRegistry)
		{
			if (value.type() == typeid(float))
			{
				DWORD valueType{};
				DWORD dataSize = sizeof(float);
				float buffer = 0.f;

				RegGetValueA(regKey, nullptr, key.c_str(), RRF_RT_ANY, &valueType, (PVOID)&buffer, &dataSize);
				myRegistry[key] = buffer;
			}
			else if (value.type() == typeid(int32_t))
			{
				DWORD valueType{};
				DWORD dataSize = sizeof(int32_t);
				int32_t buffer = 0;

				RegGetValueA(regKey, nullptr, key.c_str(), RRF_RT_ANY, &valueType, (PVOID)&buffer, &dataSize);
				myRegistry[key] = buffer;
			}
			else if (value.type() == typeid(std::string))
			{
				DWORD valueType{};
				auto* buffer = new char[512];
				DWORD dataSize = 512;

				RegGetValueA(regKey, nullptr, key.c_str(), RRF_RT_ANY, &valueType, (PVOID)buffer, &dataSize);
				myRegistry[key] = std::string(buffer);

				delete[] buffer;
			}
		}
	}

	void SessionPreferences::DeleteAll()
	{
		myRegistry.clear();
	}

	void SessionPreferences::DeleteKey(const std::string& key)
	{
		if (myRegistry.contains(key))
		{
			myRegistry.erase(key);
		}
	}

	float SessionPreferences::GetFloat(const std::string& key)
	{
		const auto& value = myRegistry.at(key);

		if (value.type() != typeid(float))
		{
			return 0.0f;
		}

		return std::any_cast<float>(value);
	}

	int32_t SessionPreferences::GetInt(const std::string& key)
	{
		const auto& value = myRegistry.at(key);

		if (value.type() != typeid(int32_t))
		{
			return 0;
		}

		return std::any_cast<int32_t>(value);
	}

	std::string SessionPreferences::GetString(const std::string& key)
	{
		const auto& value = myRegistry.at(key);

		if (value.type() != typeid(std::string))
		{
			return {};
		}

		return std::any_cast<std::string>(value);
	}

	void SessionPreferences::SetFloat(const std::string& key, float value)
	{
		myRegistry[key] = value;
	}

	void SessionPreferences::SetInt(const std::string& key, int32_t value)
	{
		myRegistry[key] = value;
	}

	void SessionPreferences::SetString(const std::string& key, const std::string& value)
	{
		myRegistry[key] = value;
	}

	bool SessionPreferences::HasKey(const std::string& key)
	{
		return myRegistry.contains(key);
	}

	void SessionPreferences::Save()
	{
		const auto regPath = GetRegistryPath();

		HKEY regKey{};
		DWORD createdNewKey{};
		DWORD openStatus = RegCreateKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &regKey, &createdNewKey);
		if (openStatus != ERROR_SUCCESS)
		{
			return;
		}

		for (const auto& [key, value] : myRegistry)
		{
			LSTATUS setStatus;

			if (value.type() == typeid(float))
			{
				float actualVal = std::any_cast<float>(value);
				setStatus = RegSetValueExA(regKey, key.c_str(), 0, REG_BINARY, (BYTE*)&actualVal, sizeof(float));
			}
			else if (value.type() == typeid(int32_t))
			{
				int32_t actualVal = std::any_cast<int32_t>(value);
				setStatus = RegSetValueExA(regKey, key.c_str(), 0, REG_DWORD, (BYTE*)&actualVal, sizeof(int32_t));
			}
			else if (value.type() == typeid(std::string))
			{
				std::string actualVal = std::any_cast<std::string>(value);
				setStatus = RegSetValueExA(regKey, key.c_str(), 0, REG_SZ, (LPBYTE)actualVal.c_str(), (DWORD)actualVal.length() + 1);
			}
		}

		RegCloseKey(regKey);
	}

	const std::string SessionPreferences::GetRegistryPath()
	{
		std::string regPath;

		if (Application::Get().IsRuntime())
		{
			regPath = "Software\\" + ProjectManager::GetProject().companyName + "\\" + ProjectManager::GetProject().name;
		}
		else
		{
			regPath = "Software\\" + std::string("Volt\\") + ProjectManager::GetProject().companyName + "\\" + ProjectManager::GetProject().name;
		}

		return regPath;
	}
}
