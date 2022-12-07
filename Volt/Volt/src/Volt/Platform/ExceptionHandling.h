#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Volt/Core/Application.h"
#include "Volt/Utility/FileSystem.h"

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>

namespace Volt
{
	inline std::filesystem::path CreateOrGetDumpFolder()
	{
		if (!FileSystem::Exists(FileSystem::GetDebugDumpPath()))
		{
			std::filesystem::create_directories(FileSystem::GetDebugDumpPath());
		}

		return FileSystem::GetDebugDumpPath();
	}

	inline static void CreateDump(EXCEPTION_POINTERS* someExceptionPointers, std::filesystem::path& lastCrashDmp)
	{
		SYSTEMTIME localTime;
		GetLocalTime(&localTime);

		const auto& appInfo = Application::Get().GetInfo();
		const std::filesystem::path dumpPath = CreateOrGetDumpFolder() / (std::format("{0}-{1}_{2}-{3}-{4}_{5}-{6}-{7}",
			appInfo.title, appInfo.version,
			localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour,
			localTime.wMinute, localTime.wSecond) + ".dmp");

		HANDLE dumpFile = CreateFile(dumpPath.wstring().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

		MINIDUMP_EXCEPTION_INFORMATION expParam;
		expParam.ThreadId = GetCurrentThreadId();
		expParam.ExceptionPointers = someExceptionPointers;
		expParam.ClientPointers = true;
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, MiniDumpWithDataSegs, &expParam, NULL, NULL);

		lastCrashDmp = dumpPath;
	}

	inline static LONG WINAPI ExceptionFilterFunction(_EXCEPTION_POINTERS* aExceptionPointer, std::filesystem::path& lastCrashDmp)
	{
		CreateDump(aExceptionPointer, lastCrashDmp);
		return EXCEPTION_EXECUTE_HANDLER;
	}
}