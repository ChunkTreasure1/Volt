#pragma once
#ifdef EVENTSYSTEMMODULE_DLL_EXPORT
#define EVENTMODULE_API __declspec(dllexport)
#else
#define EVENTMODULE_API __declspec(dllimport)
#endif
