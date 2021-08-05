#pragma once

#ifdef SOCKET_EXPORTS
#define SOCKET_API	extern "C" __declspec(dllexport)
#else
#define SOCKET_API	extern "C" __declspec(dllimport)
#endif