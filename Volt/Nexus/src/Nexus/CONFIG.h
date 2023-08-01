#pragma once

#pragma region Values
#define NXS_API_WINSOCK 0
#define NXS_API_ASIO 1

#define NXS_PROTOCOL_UDP 0
#define NXS_PROTOCOL_UDP_NONBLOCKING 1
#define NXS_PROTOCOL_TCP 2

#define NXS_MODULE_CORE (1<<0)
#define NXS_MODULE_HOOKS (1<<1)

#define NXS_MODULE_ALL (NXS_MODULE_CORE | NXS_MODULE_HOOKS)
#pragma endregion

#pragma region Build configuration dependent
#ifdef _WINSOCKAPI_
#define NXS_API_CONFIG NXS_API_WINSOCK
#endif
#pragma endregion

#pragma region Manual configuration
#define NXS_PROTOCOL NXS_PROTOCOL_UDP
#define NXS_MODULES NXS_MODULE_ALL
#pragma endregion
