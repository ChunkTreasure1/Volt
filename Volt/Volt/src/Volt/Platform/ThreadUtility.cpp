#include "vtpch.h"
#include "ThreadUtility.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.  
	LPCSTR szName; // Pointer to name (in user addr space).  
	DWORD dwThreadID; // Thread ID (-1=caller thread).  
	DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;
#pragma pack(pop)  

#pragma warning(push)  
#pragma warning(disable : 4311 4302)
void SetThreadName(std::thread::native_handle_type dwThreadID, const char* threadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = GetThreadId(static_cast<HANDLE>(dwThreadID));
	info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#pragma warning(pop)  
}
#pragma warning(pop)  
