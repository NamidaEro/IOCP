#pragma once

#include "InnInclude.h"
#include "Define.h"
#include "Functions.h"
#include "Singleton.h"

template<class T> struct TSimpleList
{
public:
	T* PushFront(T* pItem)
	{
		if (pFront != nullptr)
		{
			pFront->last = pItem;
			pItem->next = pFront;
		}
		else
		{
			pItem->last = nullptr;
			pItem->next = nullptr;
			pBack = pItem;
		}

		pFront = pItem;
		++size;

		return pItem;
	}

	T* PushBack(T* pItem)
	{
		if (pBack != nullptr)
		{
			pBack->next = pItem;
			pItem->last = pBack;
		}
		else
		{
			pItem->last = nullptr;
			pItem->next = nullptr;
			pFront = pItem;
		}

		pBack = pItem;
		++size;

		return pItem;
	}

	T* PopFront()
	{
		T* pItem = pFront;

		if (pFront != pBack)
		{
			pFront = pFront->next;
			pFront->last = nullptr;
		}
		else if (pFront != nullptr)
		{
			pFront = nullptr;
			pBack = nullptr;
		}

		if (pItem != nullptr)
		{
			pItem->next = nullptr;
			pItem->last = nullptr;

			--size;
		}

		return pItem;
	}

	T* PopBack()
	{
		T* pItem = pBack;

		if (pFront != pBack)
		{
			pBack = pBack->last;
			pBack->next = nullptr;
		}
		else if (pBack != nullptr)
		{
			pFront = nullptr;
			pBack = nullptr;
		}

		if (pItem != nullptr)
		{
			pItem->next = nullptr;
			pItem->last = nullptr;

			--size;
		}

		return pItem;
	}

	TSimpleList<T>& Shift(TSimpleList<T>& other)
	{
		if (&other != this && other.size > 0)
		{
			if (size > 0)
			{
				pBack->next = other.pFront;
				other.pFront->last = pBack;
			}
			else
			{
				pFront = other.pFront;
			}

			pBack = other.pBack;
			size += other.size;

			other.Reset();
		}

		return *this;
	}

	void Clear()
	{
		if (size > 0)
		{
			T* pItem;
			while ((pItem = PopFront()) != nullptr)
				T::Destruct(pItem);
		}
	}

	T* Front()	const { return pFront; }
	T* Back()	const { return pBack; }
	int		Size()	const { return size; }
	bool	IsEmpty()	const { return size == 0; }

public:
	TSimpleList() { Reset(); }
	~TSimpleList() { Clear(); }

	DECLARE_NO_COPY_CLASS(TSimpleList<T>)

private:
	void Reset()
	{
		pFront = nullptr;
		pBack = nullptr;
		size = 0;
	}

private:
	int	size;
	T* pFront;
	T* pBack;
};

class CPrivateHeapImpl
{
public:
	PVOID Alloc(SIZE_T dwSize, DWORD dwFlags = 0)
	{
		return ::HeapAlloc(m_hHeap, dwFlags, dwSize);
	}

	PVOID ReAlloc(PVOID pvMemory, SIZE_T dwSize, DWORD dwFlags = 0)
	{
		return ::HeapReAlloc(m_hHeap, dwFlags, pvMemory, dwSize);
	}

	SIZE_T Size(PVOID pvMemory, DWORD dwFlags = 0)
	{
		return ::HeapSize(m_hHeap, dwFlags, pvMemory);
	}

	BOOL Free(PVOID pvMemory, DWORD dwFlags = 0)
	{
		return ::HeapFree(m_hHeap, dwFlags, pvMemory);
	}

	SIZE_T Compact(DWORD dwFlags = 0)
	{
		return ::HeapCompact(m_hHeap, dwFlags);
	}

	BOOL IsValid() { return m_hHeap != nullptr; }

	BOOL Reset()
	{
		if (IsValid()) ::HeapDestroy(m_hHeap);
		m_hHeap = ::HeapCreate(m_dwOptions, m_dwInitSize, m_dwMaxSize);

		return IsValid();
	}

public:
	CPrivateHeapImpl(DWORD dwOptions = 0, SIZE_T dwInitSize = 0, SIZE_T dwMaxSize = 0)
		: m_dwOptions(dwOptions | HEAP_GENERATE_EXCEPTIONS), m_dwInitSize(dwInitSize), m_dwMaxSize(dwMaxSize)
	{
		m_hHeap = ::HeapCreate(m_dwOptions, m_dwInitSize, m_dwMaxSize);
		ENSURE(IsValid());
	}

	~CPrivateHeapImpl() { if (IsValid()) ::HeapDestroy(m_hHeap); }

	operator HANDLE	() { return m_hHeap; }

private:
	CPrivateHeapImpl(const CPrivateHeapImpl&);
	CPrivateHeapImpl operator = (const CPrivateHeapImpl&);

private:
	HANDLE	m_hHeap;
	DWORD	m_dwOptions;
	SIZE_T	m_dwInitSize;
	SIZE_T	m_dwMaxSize;
};

typedef CPrivateHeapImpl	CPrivateHeap;

template <class T> class CRingPool
{
private:

	typedef T* TPTR;
	typedef volatile T* VTPTR;

	static TPTR const E_EMPTY;
	static TPTR const E_LOCKED;
	static TPTR const E_MAX_STATUS;

private:

	VTPTR& INDEX_VAL(DWORD dwIndex) { return *(m_pv + dwIndex); }

public:

	BOOL TryPut(TPTR pElement)
	{
		ASSERT(pElement != nullptr);

		if (!IsValid()) return FALSE;

		BOOL isOK = FALSE;

		for (DWORD i = 0; i < m_dwSize; i++)
		{
			DWORD seqPut = m_seqPut;

			if (!HasPutSpace(seqPut))
				break;

			DWORD dwIndex = seqPut % m_dwSize;
			VTPTR& pValue = INDEX_VAL(dwIndex);
			TPTR pCurrent = (TPTR)pValue;

			if (pCurrent == E_EMPTY)
			{
				if (::InterlockedCompareExchangePointer((volatile PVOID*)&pValue, pElement, pCurrent) == pCurrent)
				{
					::InterlockedCompareExchange(&m_seqPut, seqPut + 1, seqPut);

					isOK = TRUE;

					break;
				}
			}

			::InterlockedCompareExchange(&m_seqPut, seqPut + 1, seqPut);
		}

		return isOK;
	}

	BOOL TryGet(TPTR* ppElement)
	{
		ASSERT(ppElement != nullptr);

		if (!IsValid()) return FALSE;

		BOOL isOK = FALSE;

		while (true)
		{
			DWORD seqGet = m_seqGet;

			if (!HasGetSpace(seqGet))
				break;

			DWORD dwIndex = seqGet % m_dwSize;
			VTPTR& pValue = INDEX_VAL(dwIndex);
			TPTR pCurrent = (TPTR)pValue;

			if (pCurrent > E_MAX_STATUS)
			{
				if (::InterlockedCompareExchangePointer((volatile PVOID*)&pValue, E_EMPTY, pCurrent) == pCurrent)
				{
					::InterlockedCompareExchange(&m_seqGet, seqGet + 1, seqGet);

					*(ppElement) = pCurrent;
					isOK = TRUE;

					break;
				}
			}

			::InterlockedCompareExchange(&m_seqGet, seqGet + 1, seqGet);
		}

		return isOK;
	}

	BOOL TryLock(TPTR* ppElement, DWORD& dwIndex)
	{
		ASSERT(ppElement != nullptr);

		if (!IsValid()) return FALSE;

		BOOL isOK = FALSE;

		while (true)
		{
			DWORD seqGet = m_seqGet;

			if (!HasGetSpace(seqGet))
				break;

			dwIndex = seqGet % m_dwSize;
			VTPTR& pValue = INDEX_VAL(dwIndex);
			TPTR pCurrent = (TPTR)pValue;

			if (pCurrent > E_MAX_STATUS)
			{
				if (::InterlockedCompareExchangePointer((volatile PVOID*)&pValue, E_LOCKED, pCurrent) == pCurrent)
				{
					::InterlockedCompareExchange(&m_seqGet, seqGet + 1, seqGet);

					*(ppElement) = pCurrent;
					isOK = TRUE;

					break;
				}
			}

			::InterlockedCompareExchange(&m_seqGet, seqGet + 1, seqGet);
		}

		return isOK;
	}

	BOOL ReleaseLock(TPTR pElement, DWORD dwIndex)
	{
		ASSERT(dwIndex < m_dwSize);
		ASSERT(pElement == nullptr || pElement > E_MAX_STATUS);

		if (!IsValid()) return FALSE;

		VTPTR& pValue = INDEX_VAL(dwIndex);
		ENSURE(pValue == E_LOCKED);

		if (pElement == nullptr)
			pValue = E_EMPTY;
		else
			pValue = pElement;

		return TRUE;
	}

public:

	void Reset(DWORD dwSize = 0)
	{
		if (IsValid())
			Destroy();
		if (dwSize > 0)
			Create(dwSize);
	}

	void Clear()
	{
		for (DWORD dwIndex = 0; dwIndex < m_dwSize; dwIndex++)
		{
			VTPTR& pValue = INDEX_VAL(dwIndex);

			if (pValue > E_MAX_STATUS)
			{
				T::Destruct((TPTR)pValue);
				pValue = E_EMPTY;
			}
		}

		Reset();
	}

	DWORD Size() { return m_dwSize; }
	DWORD Elements() { return m_seqPut - m_seqGet; }
	BOOL IsFull() { return Elements() == Size(); }
	BOOL IsEmpty() { return Elements() == 0; }
	BOOL IsValid() { return m_pv != nullptr; }

private:

	BOOL HasPutSpace(DWORD seqPut)
	{
		return ((int)(seqPut - m_seqGet) < (int)m_dwSize);
	}

	BOOL HasGetSpace(DWORD seqGet)
	{
		return ((int)(m_seqPut - seqGet) > 0);
	}

	void Create(DWORD dwSize)
	{
		ASSERT(!IsValid() && dwSize > 0);

		m_seqPut = 0;
		m_seqGet = 0;
		m_dwSize = dwSize;
		m_pv = (VTPTR*)malloc(m_dwSize * sizeof(TPTR));

		::ZeroMemory(m_pv, m_dwSize * sizeof(TPTR));
	}

	void Destroy()
	{
		ASSERT(IsValid());

		free((void*)m_pv);
		m_pv = nullptr;
		m_dwSize = 0;
		m_seqPut = 0;
		m_seqGet = 0;
	}

public:
	CRingPool(DWORD dwSize = 0)
		: m_pv(nullptr)
		, m_dwSize(0)
		, m_seqPut(0)
		, m_seqGet(0)
	{
		Reset(dwSize);
	}

	~CRingPool()
	{
		Reset(0);
	}

private:
	CRingPool(const CRingPool&);
	CRingPool operator = (const CRingPool&);

private:
	DWORD				m_dwSize;
	VTPTR* m_pv;
	char				pack1[PACK_SIZE_OF(VTPTR*)];
	volatile DWORD		m_seqPut;
	char				pack2[PACK_SIZE_OF(DWORD)];
	volatile DWORD		m_seqGet;
	char				pack3[PACK_SIZE_OF(DWORD)];
};

template<class T> class CNodePoolT
{
public:
	void PutFreeItem(T* pItem)
	{
		ASSERT(pItem != nullptr);

		if (!m_lsFreeItem.TryPut(pItem))
			T::Destruct(pItem);
	}

	void PutFreeItem(TSimpleList<T>& lsItem)
	{
		if (lsItem.IsEmpty())
			return;

		T* pItem;
		while ((pItem = lsItem.PopFront()) != nullptr)
			PutFreeItem(pItem);
	}

	T* PickFreeItem()
	{
		T* pItem = nullptr;

		if (!m_lsFreeItem.TryGet(&pItem))
			pItem = T::Construct(m_heap, m_dwItemCapacity);

		ASSERT(pItem);
		pItem->Reset();

		return pItem;
	}

	void Prepare()
	{
		m_lsFreeItem.Reset(m_dwPoolSize);
	}

	void Clear()
	{
		m_lsFreeItem.Clear();

		m_heap.Reset();
	}

public:
	void SetItemCapacity(DWORD dwItemCapacity) { m_dwItemCapacity = dwItemCapacity; }
	void SetPoolSize(DWORD dwPoolSize) { m_dwPoolSize = dwPoolSize; }
	void SetPoolHold(DWORD dwPoolHold) { m_dwPoolHold = dwPoolHold; }
	DWORD GetItemCapacity() { return m_dwItemCapacity; }
	DWORD GetPoolSize() { return m_dwPoolSize; }
	DWORD GetPoolHold() { return m_dwPoolHold; }

public:
	CNodePoolT(DWORD dwPoolSize = DEFAULT_POOL_SIZE,
		DWORD dwPoolHold = DEFAULT_POOL_HOLD,
		DWORD dwItemCapacity = DEFAULT_ITEM_CAPACITY)
		: m_dwPoolSize(dwPoolSize)
		, m_dwPoolHold(dwPoolHold)
		, m_dwItemCapacity(dwItemCapacity)
	{
	}

	~CNodePoolT() { Clear(); }

	DECLARE_NO_COPY_CLASS(CNodePoolT)

public:
	static const DWORD DEFAULT_ITEM_CAPACITY;
	static const DWORD DEFAULT_POOL_SIZE;
	static const DWORD DEFAULT_POOL_HOLD;

private:
	CPrivateHeap	m_heap;

	DWORD			m_dwItemCapacity;
	DWORD			m_dwPoolSize;
	DWORD			m_dwPoolHold;

	CRingPool<T>	m_lsFreeItem;
};

class CSpinGuard
{
public:
	CSpinGuard() : m_lFlag(0)
	{

	}

	~CSpinGuard()
	{
		ASSERT(m_lFlag == 0);
	}

	void Lock()
	{
		for (UINT i = 0; !TryLock(); ++i)
			YieldThread(i);
	}

	BOOL TryLock()
	{
		if (::InterlockedCompareExchange(&m_lFlag, 1, 0) == 0)
		{
			::_ReadWriteBarrier();
			return TRUE;
		}

		return FALSE;
	}

	void Unlock()
	{
		ASSERT(m_lFlag == 1);
		m_lFlag = 0;
	}

private:
	CSpinGuard(const CSpinGuard& cs);
	CSpinGuard operator = (const CSpinGuard& cs);

private:
	volatile LONG m_lFlag;
};

class CInterCriSec
{
public:
	CInterCriSec(DWORD dwSpinCount = DEFAULT_CRISEC_SPIN_COUNT)
	{
		ENSURE(::InitializeCriticalSectionAndSpinCount(&m_crisec, dwSpinCount));
	}
	~CInterCriSec()
	{
		::DeleteCriticalSection(&m_crisec);
	}

	void Lock() { ::EnterCriticalSection(&m_crisec); }
	void Unlock() { ::LeaveCriticalSection(&m_crisec); }
	BOOL TryLock() { return ::TryEnterCriticalSection(&m_crisec); }
	DWORD SetSpinCount(DWORD dwSpinCount) { return ::SetCriticalSectionSpinCount(&m_crisec, dwSpinCount); }

	CRITICAL_SECTION* GetObject() { return &m_crisec; }

private:
	CInterCriSec(const CInterCriSec& cs);
	CInterCriSec operator = (const CInterCriSec& cs);

private:
	CRITICAL_SECTION m_crisec;
};

template<class CLockObj> class CLocalLock
{
public:
	CLocalLock(CLockObj& obj) : m_lock(obj) { m_lock.Lock(); }
	~CLocalLock() { m_lock.Unlock(); }
private:
	CLockObj& m_lock;
};

template<class T> struct TBufferObjBase
{
	WSAOVERLAPPED		ov;
	CPrivateHeap& heap;

	EnSocketOperation	operation;
	WSABUF				buff;

	int					capacity;
	volatile LONG		sndCounter;

	T* next;
	T* last;

	static T* Construct(CPrivateHeap& heap, DWORD dwCapacity)
	{
		T* pBufferObj = (T*)heap.Alloc(sizeof(T) + dwCapacity);
		ASSERT(pBufferObj);

		pBufferObj->TBufferObjBase::TBufferObjBase(heap, dwCapacity);
		pBufferObj->buff.buf = ((char*)pBufferObj) + sizeof(T);

		return pBufferObj;
	}

	static void Destruct(T* pBufferObj)
	{
		ASSERT(pBufferObj);
		pBufferObj->heap.Free(pBufferObj);
	}

	void ResetSendCounter()
	{
		sndCounter = 2;
	}

	LONG ReleaseSendCounter()
	{
		return ::InterlockedDecrement(&sndCounter);
	}

	TBufferObjBase(CPrivateHeap& hp, DWORD dwCapacity)
		: heap(hp)
		, capacity((int)dwCapacity)
	{
		ASSERT(capacity > 0);
	}

	int Cat(const BYTE* pData, int length)
	{
		ASSERT(pData != nullptr && length >= 0);

		int cat = min(Remain(), length);

		if (cat > 0)
		{
			memcpy(buff.buf + buff.len, pData, cat);
			buff.len += cat;
		}

		return cat;
	}

	void ResetOV() { ::ZeroMemory(&ov, sizeof(ov)); }
	void Reset() { ResetOV(); buff.len = 0; }
	int Remain() { return capacity - buff.len; }
	BOOL IsFull() { return Remain() == 0; }
};

struct TBufferObj : public TBufferObjBase<TBufferObj>
{
	SOCKET client;
};

template<class T> struct TBufferObjListT : public TSimpleList<T>
{
public:
	int Cat(const BYTE* pData, int length)
	{
		ASSERT(pData != nullptr && length >= 0);

		int remain = length;

		while (remain > 0)
		{
			T* pItem = TBufferObjListT::Back();

			if (pItem == nullptr || pItem->IsFull())
				pItem = PushBack(bfPool.PickFreeItem());

			int cat = pItem->Cat(pData, remain);

			pData += cat;
			remain -= cat;
		}

		return length;
	}

	T* PushTail(const BYTE* pData, int length)
	{
		ASSERT(pData != nullptr && length >= 0 && length <= (int)bfPool.GetItemCapacity());

		T* pItem = PushBack(bfPool.PickFreeItem());
		pItem->Cat(pData, length);

		return pItem;
	}

	void Release()
	{
		bfPool.PutFreeItem(*this);
	}

public:
	TBufferObjListT(CNodePoolT<T>& pool) : bfPool(pool)
	{
	}

private:
	CNodePoolT<T>& bfPool;
};

typedef TBufferObjListT<TBufferObj>		TBufferObjList;
typedef CInterCriSec CCriSec;
typedef CLocalLock<CCriSec>					CCriSecLock;
typedef CNodePoolT<TBufferObj>			CBufferObjPool;

typedef struct _sockaddr
{
	union
	{
		ADDRESS_FAMILY	family;
		SOCKADDR		addr;
		SOCKADDR_IN		addr4;
		SOCKADDR_IN6	addr6;
	};

	inline int AddrSize() const
	{
		return AddrSize(family);
	}

	inline static int AddrSize(ADDRESS_FAMILY f)
	{
		if (f == AF_INET)
			return sizeof(SOCKADDR_IN);

		return sizeof(SOCKADDR_IN6);
	}

	inline int EffectAddrSize() const
	{
		return EffectAddrSize(family);
	}

	inline static int EffectAddrSize(ADDRESS_FAMILY f)
	{
		return (f == AF_INET) ? offsetof(SOCKADDR_IN, sin_zero) : sizeof(SOCKADDR_IN6);
	}

	inline static const _sockaddr& AnyAddr(ADDRESS_FAMILY f)
	{
		static const _sockaddr s_any_addr4(AF_INET, TRUE);
		static const _sockaddr s_any_addr6(AF_INET6, TRUE);

		if (f == AF_INET)
			return s_any_addr4;

		return s_any_addr6;
	}

	inline static int AddrMinStrLength(ADDRESS_FAMILY f)
	{
		if (f == AF_INET)
			return 16;

		return 46;
	}

	inline BOOL IsIPv4()			const { return family == AF_INET; }
	inline BOOL IsIPv6()			const { return family == AF_INET6; }
	inline BOOL IsSpecified()		const { return IsIPv4() || IsIPv6(); }
	inline USHORT Port()			const { return ntohs(addr4.sin_port); }
	inline void SetPort(USHORT usPort) { addr4.sin_port = htons(usPort); }
	inline void* SinAddr()			const { return IsIPv4() ? (void*)&addr4.sin_addr : (void*)&addr6.sin6_addr; }
	inline void* SinAddr() { return IsIPv4() ? (void*)&addr4.sin_addr : (void*)&addr6.sin6_addr; }

	inline const SOCKADDR* Addr()	const { return &addr; }
	inline SOCKADDR* Addr() { return &addr; }
	inline void ZeroAddr() { ::ZeroMemory(((char*)this) + sizeof(family), sizeof(*this) - sizeof(family)); }
	inline void Reset() { ::ZeroMemory(this, sizeof(*this)); }

	inline _sockaddr& Copy(_sockaddr& other) const
	{
		if (this != &other)
			memcpy(&other, this, AddrSize());

		return other;
	}

	size_t Hash() const
	{
		ASSERT(IsSpecified());

		size_t _Val = 2166136261U;
		const int size = EffectAddrSize();
		const BYTE* pAddr = (const BYTE*)Addr();

		for (int i = 0; i < size; i++)
			_Val = 16777619U * _Val ^ (size_t)pAddr[i];

		return (_Val);
	}

	bool EqualTo(const _sockaddr& other) const
	{
		ASSERT(IsSpecified() && other.IsSpecified());

		return EqualMemory(this, &other, EffectAddrSize());
	}

	_sockaddr(ADDRESS_FAMILY f = AF_UNSPEC, BOOL bZeroAddr = FALSE)
	{
		family = f;

		if (bZeroAddr) ZeroAddr();
	}

} _SOCKADDR, *_PSOCKADDR;

struct TSocketObjBase
{
	CPrivateHeap& heap;

	ULONG_PTR	connID;
	_SOCKADDR	remoteAddr;
	PVOID		extra;
	PVOID		reserved;
	PVOID		reserved2;
	BOOL		valid;

	union
	{
		DWORD	freeTime;
		DWORD	connTime;
	};

	DWORD		activeTime;

	volatile BOOL	smooth;
	volatile long	pending;
	volatile long	sndCount;

	volatile BOOL	connected;
	volatile BOOL	paused;
	volatile BOOL	recving;

	TSocketObjBase(CPrivateHeap& hp) : heap(hp) {}

	static BOOL IsExist(TSocketObjBase* pSocketObj)
	{
		return pSocketObj != nullptr;
	}

	static BOOL IsValid(TSocketObjBase* pSocketObj)
	{
		return pSocketObj != nullptr && pSocketObj->valid;
	}

	static void Invalid(TSocketObjBase* pSocketObj)
	{
		ASSERT(IsExist(pSocketObj)); pSocketObj->valid = FALSE;
	}

	static void Release(TSocketObjBase* pSocketObj)
	{
		ASSERT(IsExist(pSocketObj)); pSocketObj->freeTime = ::TimeGetTime();
	}

	DWORD GetConnTime()	const { return connTime; }
	DWORD GetFreeTime()	const { return freeTime; }
	DWORD GetActiveTime()	const { return activeTime; }
	BOOL IsPaused()	const { return paused; }

	long Pending() { return pending; }
	BOOL IsPending() { return pending > 0; }
	BOOL IsSmooth() { return smooth; }
	void TurnOnSmooth() { smooth = TRUE; }

	BOOL TurnOffSmooth()
	{
		return ::InterlockedCompareExchange((volatile long*)&smooth, FALSE, TRUE) == TRUE;
	}

	BOOL HasConnected() { return connected; }
	void SetConnected(BOOL bConnected = TRUE) { connected = bConnected; }

	void Reset(ULONG_PTR dwConnID)
	{
		connID = dwConnID;
		connected = FALSE;
		valid = TRUE;
		smooth = TRUE;
		paused = FALSE;
		recving = FALSE;
		pending = 0;
		sndCount = 0;
		extra = nullptr;
		reserved = nullptr;
		reserved2 = nullptr;
	}
};

struct TSocketObj : public TSocketObjBase
{
	CCriSec			csRecv;
	CCriSec			csSend;
	CSpinGuard		sgPause;

	SOCKET			socket;
	CStringA		host;
	TBufferObjList	sndBuff;

	BOOL IsCanSend() { return sndCount <= GetSendBufferSize(); }

	long GetSendBufferSize()
	{
		long lSize;
		int len = (int)(sizeof(lSize));
		int rs = getsockopt(socket, SOL_SOCKET, SO_SNDBUF, (CHAR*)&lSize, &len);

		if (rs == SOCKET_ERROR || lSize <= 0)
			lSize = DEFAULT_SOCKET_SNDBUFF_SIZE;

		return lSize;
	}

	static TSocketObj* Construct(CPrivateHeap& hp, CBufferObjPool& bfPool)
	{
		TSocketObj* pSocketObj = (TSocketObj*)hp.Alloc(sizeof(TSocketObj));
		ASSERT(pSocketObj);

		pSocketObj->TSocketObj::TSocketObj(hp, bfPool);

		return pSocketObj;
	}

	static void Destruct(TSocketObj* pSocketObj)
	{
		ASSERT(pSocketObj);

		CPrivateHeap& heap = pSocketObj->heap;
		pSocketObj->TSocketObj::~TSocketObj();
		heap.Free(pSocketObj);
	}

	TSocketObj(CPrivateHeap& hp, CBufferObjPool& bfPool)
		: TSocketObjBase(hp), sndBuff(bfPool)
	{

	}

	static BOOL InvalidSocketObj(TSocketObj* pSocketObj)
	{
		BOOL bDone = FALSE;

		if (TSocketObj::IsValid(pSocketObj))
		{
			pSocketObj->SetConnected(FALSE);

			CCriSecLock locallock(pSocketObj->csRecv);
			CCriSecLock locallock2(pSocketObj->csSend);

			if (TSocketObjBase::IsValid(pSocketObj))
			{
				TSocketObjBase::Invalid(pSocketObj);
				bDone = TRUE;
			}
		}

		return bDone;
	}

	static void Release(TSocketObj* pSocketObj)
	{
		__super::Release(pSocketObj);

		pSocketObj->sndBuff.Release();
	}

	void Reset(ULONG_PTR dwConnID, SOCKET soClient)
	{
		__super::Reset(dwConnID);

		host.Empty();

		socket = soClient;
	}

	BOOL GetRemoteHost(LPCSTR* lpszHost, USHORT* pusPort = nullptr)
	{
		*lpszHost = host;

		if (pusPort)
			*pusPort = remoteAddr.Port();

		return (*lpszHost != nullptr && (*lpszHost)[0] != 0);
	}
};



