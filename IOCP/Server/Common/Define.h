#pragma once

#ifdef SOCKET_EXPORTS
#define SOCKET_API	EXTERN_C __declspec(dllexport)
#else
#define SOCKET_API	EXTERN_C __declspec(dllimport)
#endif

#define DEFAULT_CRISEC_SPIN_COUNT	0
#define THREAD_YIELD_CYCLE			63
#define THREAD_SWITCH_CYCLE			4095
//#define PURE = 0;

typedef enum EnServiceState
{
	SS_STARTING = 0,
	SS_STARTED = 1,
	SS_STOPPING = 2,
	SS_STOPPED = 3,
} En_ServiceState;

typedef enum EnSocketError
{
	SE_OK = 0L,
	SE_ILLEGAL_STATE = 1,
	SE_INVALID_PARAM = 2,
	SE_SOCKET_CREATE = 3,
	SE_SOCKET_BIND = 4,
	SE_SOCKET_PREPARE = 5,
	SE_SOCKET_LISTEN = 6,
	SE_CP_CREATE = 7,
	SE_WORKER_THREAD_CREATE = 8,
	SE_DETECT_THREAD_CREATE = 9,
	SE_SOCKE_ATTACH_TO_CP = 10,
	SE_CONNECT_SERVER = 11,
	SE_NETWORK = 12,
	SE_DATA_PROC = 13,
	SE_DATA_SEND = 14,

	SE_SSL_ENV_NOT_READY = 101,
} En_SocketError;

typedef enum EnReuseAddressPolicy
{
	RAP_NONE = 0,
	RAP_ADDR_ONLY = 1,
	RAP_ADDR_AND_PORT = 2,
} En_ReuseAddressPolicy;

typedef enum EnSendPolicy
{
	SP_PACK = 0,
	SP_SAFE = 1,
	SP_DIRECT = 2,
} En_SendPolicy;

typedef enum EnOnSendSyncPolicy
{
	OSSP_NONE = 0,
	OSSP_CLOSE = 1,
	OSSP_RECEIVE = 2,
} En_OnSendSyncPolicy;

typedef enum EnHandleResult
{
	HR_OK = 0,
	HR_IGNORE = 1,
	HR_ERROR = 2,
} En_HandleResult;

typedef enum EnSocketOperation
{
	SO_UNKNOWN = 0,	// Unknown
	SO_ACCEPT = 1,	// Acccept
	SO_CONNECT = 2,	// Connect
	SO_SEND = 3,	// Send
	SO_RECEIVE = 4,	// Receive
	SO_CLOSE = 5,	// Close
} En_SocketOperation;

enum EnSocketCloseFlag
{
	SCF_NONE = 0,
	SCF_CLOSE = 1,
	SCF_ERROR = 2
};

enum EnIocpAction
{
	IOCP_ACT_GOON = 0,
	IOCP_ACT_CONTINUE = 1,
	IOCP_ACT_BREAK = 2
};

#define Min(a,b)            (((a) < (b)) ? (a) : (b))

#define EqualMemory(dest, src, len)		(!memcmp((dest), (src), (len)))
#define DEFAULT_SOCKET_SNDBUFF_SIZE				(16 * 1024)
#define CACHE_LINE		64
#define PACK_SIZE_OF(T)	(CACHE_LINE - sizeof(T) % CACHE_LINE)
#define MAX_IOCP_CONTINUE_RECEIVE				30
#define MAX_WORKER_THREAD_COUNT			512
#define DEFAULT_OBJECT_CACHE_LOCK_TIME	(20 * 1000)
#define DEFAULT_OBJECT_CACHE_POOL_SIZE	600
#define DEFAULT_OBJECT_CACHE_POOL_HOLD	600
#define DEFAULT_BUFFER_CACHE_CAPACITY	4096
#define DEFAULT_BUFFER_CACHE_POOL_SIZE	1024
#define DEFAULT_BUFFER_CACHE_POOL_HOLD	1024

#define DEFAULT_TCP_SERVER_SOCKET_LISTEN_QUEUE	SOMAXCONN
#define DEFAULT_TCP_SERVER_ACCEPT_SOCKET_COUNT	300
#define DEFAULT_TCP_SOCKET_BUFFER_SIZE			4096
#define DEFAULT_FREE_SOCKETOBJ_LOCK_TIME		DEFAULT_OBJECT_CACHE_LOCK_TIME
#define DEFAULT_FREE_SOCKETOBJ_POOL				DEFAULT_OBJECT_CACHE_POOL_SIZE
#define DEFAULT_FREE_SOCKETOBJ_HOLD				DEFAULT_OBJECT_CACHE_POOL_HOLD
#define DEFAULT_FREE_BUFFEROBJ_POOL				DEFAULT_BUFFER_CACHE_POOL_SIZE
#define DEFAULT_FREE_BUFFEROBJ_HOLD				DEFAULT_BUFFER_CACHE_POOL_HOLD

#define DEFALUT_TCP_KEEPALIVE_TIME				(60 * 1000)
#define DEFALUT_TCP_KEEPALIVE_INTERVAL			(20 * 1000)

#define CreateLocalObjects(T, n)		((T*)alloca(sizeof(T) * (n)))
#define CreateLocalObject(T)			CreateLocalObjects(T, 1)