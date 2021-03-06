/*
* Copyright: JessMA Open Source (ldcsaa@gmail.com)
*
* Author	: Bruce Liang
* Website	: https://github.com/ldcsaa
* Project	: https://github.com/ldcsaa/HP-Socket
* Blog		: http://www.cnblogs.com/ldcsaa
* Wiki		: http://www.oschina.net/p/hp-socket
* QQ Group	: 44636872, 75375912
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include "../include/hpsocket/HPTypeDef.h"
#include "../include/hpsocket/SocketInterface.h"
#include "common/StringT.h"
#include "common/SysHelper.h"
#include "common/BufferPtr.h"
#include "common/BufferPool.h"
#include "common/RingBuffer.h"
#include "common/FileHelper.h"
#include "InternalDef.h"

#include <netdb.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef _ZLIB_SUPPORT
#include <zlib.h>
#endif

#ifdef _BROTLI_SUPPORT
#include <brotli/decode.h>
#include <brotli/encode.h>
#endif

using ADDRESS_FAMILY	= sa_family_t;
using IN_ADDR			= in_addr;
using IN6_ADDR			= in6_addr;
using SOCKADDR			= sockaddr;
using SOCKADDR_IN		= sockaddr_in;
using SOCKADDR_IN6		= sockaddr_in6;

typedef struct hp_addr
{
	ADDRESS_FAMILY family;

	union
	{
		ULONG_PTR	addr;
		IN_ADDR		addr4;
		IN6_ADDR	addr6;
	};

	static const hp_addr ANY_ADDR4;
	static const hp_addr ANY_ADDR6;

	inline int AddrSize() const
	{
		return AddrSize(family);
	}

	inline static int AddrSize(ADDRESS_FAMILY f)
	{
		if(f == AF_INET)
			return sizeof(IN_ADDR);

		return sizeof(IN6_ADDR);
	}

	inline static const hp_addr& AnyAddr(ADDRESS_FAMILY f)
	{
		if(f == AF_INET)
			return ANY_ADDR4;

		return ANY_ADDR6;
	}

	inline const ULONG_PTR* Addr()	const	{return &addr;}
	inline ULONG_PTR* Addr()				{return &addr;}

	inline BOOL IsIPv4()			const	{return family == AF_INET;}
	inline BOOL IsIPv6()			const	{return family == AF_INET6;}
	inline BOOL IsSpecified()		const	{return IsIPv4() || IsIPv6();}
	inline void ZeroAddr()					{::ZeroMemory(&addr6, sizeof(addr6));}
	inline void Reset()						{::ZeroMemory(this, sizeof(*this));}

	inline hp_addr& Copy(hp_addr& other) const
	{
		if(this != &other)
			memcpy(&other, this, offsetof(hp_addr, addr) + AddrSize());

		return other;
	}

	hp_addr(ADDRESS_FAMILY f = AF_UNSPEC, BOOL bZeroAddr = FALSE)
	{
		family = f;

		if(bZeroAddr) ZeroAddr();
	}

} HP_ADDR, *HP_PADDR;

typedef struct hp_sockaddr
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
		if(f == AF_INET)
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

	inline static const hp_sockaddr& AnyAddr(ADDRESS_FAMILY f)
	{
		static const hp_sockaddr s_any_addr4(AF_INET, TRUE);
		static const hp_sockaddr s_any_addr6(AF_INET6, TRUE);

		if(f == AF_INET)
			return s_any_addr4;

		return s_any_addr6;
	}

	inline static int AddrMinStrLength(ADDRESS_FAMILY f)
	{
		if(f == AF_INET)
			return 16;

		return 46;
	}

	inline BOOL IsIPv4()			const	{return family == AF_INET;}
	inline BOOL IsIPv6()			const	{return family == AF_INET6;}
	inline BOOL IsSpecified()		const	{return IsIPv4() || IsIPv6();}
	inline USHORT Port()			const	{return ntohs(addr4.sin_port);}
	inline void SetPort(USHORT usPort)		{addr4.sin_port = htons(usPort);}
	inline void* SinAddr()			const	{return IsIPv4() ? (void*)&addr4.sin_addr : (void*)&addr6.sin6_addr;}
	inline void* SinAddr()					{return IsIPv4() ? (void*)&addr4.sin_addr : (void*)&addr6.sin6_addr;}

	inline const SOCKADDR* Addr()	const	{return &addr;}
	inline SOCKADDR* Addr()					{return &addr;}
	inline void ZeroAddr()					{::ZeroMemory(((char*)this) + sizeof(family), sizeof(*this) - sizeof(family));}
	inline void Reset()						{::ZeroMemory(this, sizeof(*this));}

	inline hp_sockaddr& Copy(hp_sockaddr& other) const
	{
		if(this != &other)
			memcpy(&other, this, AddrSize());

		return other;
	}

	size_t Hash() const
	{
		ASSERT(IsSpecified());

		size_t _Val		  = 2166136261U;
		const int size	  = EffectAddrSize();
		const BYTE* pAddr = (const BYTE*)Addr();

		for(int i = 0; i < size; i++)
			_Val = 16777619U * _Val ^ (size_t)pAddr[i];

		return (_Val);
	}

	bool EqualTo(const hp_sockaddr& other) const
	{
		ASSERT(IsSpecified() && other.IsSpecified());

		return EqualMemory(this, &other, EffectAddrSize());
	}

	hp_sockaddr(ADDRESS_FAMILY f = AF_UNSPEC, BOOL bZeroAddr = FALSE)
	{
		family = f;

		if(bZeroAddr) ZeroAddr();
	}

} HP_SOCKADDR, *HP_PSOCKADDR;

struct TNodeBufferObj : public TItem
{
	using __super = TItem;

	HP_SOCKADDR	remoteAddr;

public:
	void Reset(int first = 0, int last = 0)
	{
		__super::Reset(first, last);
		remoteAddr.Reset();
	}

public:
	static TNodeBufferObj* Construct(CPrivateHeap& heap,
									int		capacity	= DEFAULT_ITEM_CAPACITY,
									BYTE*	pData		= nullptr,
									int		length		= 0)
	{
		return ::ConstructItemT((TNodeBufferObj*)(nullptr), heap, capacity, pData, length);
	}

	static void Destruct(TNodeBufferObj* pBufferObj)
	{
		::DestructItemT(pBufferObj);
	}

	TNodeBufferObj(CPrivateHeap& hp, BYTE* pHead, int cap = DEFAULT_ITEM_CAPACITY, BYTE* pData = nullptr, int length = 0)
	: TItem(hp, pHead, cap, pData, length)
	{

	}

	~TNodeBufferObj()
	{
	}

	DECLARE_NO_COPY_CLASS(TNodeBufferObj)
};

typedef CCASQueue<TNodeBufferObj>					CNodeRecvQueue;
typedef TItemPtrT<TNodeBufferObj>					TNodeBufferObjPtr;
typedef CNodePoolT<TNodeBufferObj>					CNodeBufferObjPool;
typedef TItemListExT<TNodeBufferObj, volatile int>	TNodeBufferObjList;

/* Server ????????? Agent ????????????????????????????????????????????? */

// ???????????????
#define HR_CLOSED		0xFF

/* ???????????? */
enum EnDispCmdType
{
	DISP_CMD_SEND		= 0x01,	// ????????????
	DISP_CMD_RECEIVE	= 0x02,	// ????????????
	DISP_CMD_UNPAUSE	= 0x03,	// ??????????????????
	DISP_CMD_DISCONNECT	= 0x04,	// ????????????
	DISP_CMD_TIMEOUT	= 0x05	// ????????????
};

/* ?????????????????? */
enum EnSocketCloseFlag
{
	SCF_NONE			= 0,	// ???????????????
	SCF_CLOSE			= 1,	// ?????? ???????????? OnClose ??????
	SCF_ERROR			= 2		// ?????? ???????????? OnClose ??????
};

/* ?????????????????? */
typedef TItem			TBufferObj;
/* ?????????????????????????????? */
typedef TItemPtr		TBufferObjPtr;
/* ???????????????????????? */
typedef CItemPool		CBufferObjPool;
/* ??????????????????????????? */
typedef TItemListExV	TBufferObjList;

/* ?????? ID - ???????????????????????? */
typedef unordered_map<THR_ID, CBufferPtr*>	TReceiveBufferMap;
/* ?????? ID - ????????????????????????????????? */
typedef TReceiveBufferMap::iterator			TReceiveBufferMapI;
/* ?????? ID - ???????????????????????? const ????????? */
typedef TReceiveBufferMap::const_iterator	TReceiveBufferMapCI;

/* Socket ????????????????????? */
struct TSocketObjBase
{
	CPrivateHeap& heap;

	CONNID		connID;
	HP_SOCKADDR	remoteAddr;
	PVOID		extra;
	PVOID		reserved;
	PVOID		reserved2;
	BOOL		valid;
	DWORD		activeTime;

	union
	{
		DWORD	freeTime;
		DWORD	connTime;
	};

	volatile BOOL connected;
	volatile BOOL paused;

	TSocketObjBase(CPrivateHeap& hp) : heap(hp) {}

	static BOOL IsExist(TSocketObjBase* pSocketObj)
		{return pSocketObj != nullptr;}

	static BOOL IsValid(TSocketObjBase* pSocketObj)
		{return pSocketObj != nullptr && pSocketObj->valid;}

	static void Invalid(TSocketObjBase* pSocketObj)
		{ASSERT(IsExist(pSocketObj)); pSocketObj->valid = FALSE;}

	static void Release(TSocketObjBase* pSocketObj)
		{ASSERT(IsExist(pSocketObj)); pSocketObj->freeTime = ::TimeGetTime();}

	DWORD GetConnTime	()	const	{return connTime;}
	DWORD GetFreeTime	()	const	{return freeTime;}
	DWORD GetActiveTime	()	const	{return activeTime;}
	BOOL IsPaused		()	const	{return paused;}

	BOOL HasConnected()							{return connected == TRUE;}
	BOOL IsConnecting()							{return connected == CST_CONNECTING;}
	void SetConnected(BOOL bConnected = TRUE)	{connected = bConnected;}

	void Reset(CONNID dwConnID)
	{
		connID		= dwConnID;
		connected	= FALSE;
		valid		= TRUE;
		paused		= FALSE;
		extra		= nullptr;
		reserved	= nullptr;
		reserved2	= nullptr;
	}
};

/* ????????????????????? */
struct TSocketObj : public TSocketObjBase
{
	using __super = TSocketObjBase;

	CReentrantCriSec	csIo;
	CReentrantCriSec	csSend;

	SOCKET				socket;
	TBufferObjList		sndBuff;

	static TSocketObj* Construct(CPrivateHeap& hp, CBufferObjPool& bfPool)
	{
		TSocketObj* pSocketObj = (TSocketObj*)hp.Alloc(sizeof(TSocketObj));
		ASSERT(pSocketObj);

		return new (pSocketObj) TSocketObj(hp, bfPool);
	}

	static void Destruct(TSocketObj* pSocketObj)
	{
		ASSERT(pSocketObj);

		CPrivateHeap& heap = pSocketObj->heap;
		pSocketObj->TSocketObj::~TSocketObj();
		heap.Free(pSocketObj);
	}
	
	TSocketObj(CPrivateHeap& hp, CBufferObjPool& bfPool)
	: __super(hp), sndBuff(bfPool)
	{

	}

	static void Release(TSocketObj* pSocketObj)
	{
		__super::Release(pSocketObj);
		pSocketObj->sndBuff.Release();
	}

	int Pending()		{return sndBuff.Length();}
	BOOL IsPending()	{return Pending() > 0;}

	static BOOL InvalidSocketObj(TSocketObj* pSocketObj)
	{
		BOOL bDone = FALSE;

		if(TSocketObjBase::IsValid(pSocketObj))
		{
			pSocketObj->SetConnected(FALSE);

			CReentrantCriSecLock locallock(pSocketObj->csIo);
			CReentrantCriSecLock locallock2(pSocketObj->csSend);

			if(TSocketObjBase::IsValid(pSocketObj))
			{
				TSocketObjBase::Invalid(pSocketObj);
				bDone = TRUE;
			}
		}

		return bDone;
	}

	void Reset(CONNID dwConnID, SOCKET soClient)
	{
		__super::Reset(dwConnID);
		
		socket = soClient;
	}
};

/* Agent ????????????????????? */
struct TAgentSocketObj : public TSocketObj
{
	using __super = TSocketObj;

	CStringA host;
	
	static TAgentSocketObj* Construct(CPrivateHeap& hp, CBufferObjPool& bfPool)
	{
		TAgentSocketObj* pSocketObj = (TAgentSocketObj*)hp.Alloc(sizeof(TAgentSocketObj));
		ASSERT(pSocketObj);

		return new (pSocketObj) TAgentSocketObj(hp, bfPool);
	}

	static void Destruct(TAgentSocketObj* pSocketObj)
	{
		ASSERT(pSocketObj);

		CPrivateHeap& heap = pSocketObj->heap;
		pSocketObj->TAgentSocketObj::~TAgentSocketObj();
		heap.Free(pSocketObj);
	}
	
	TAgentSocketObj(CPrivateHeap& hp, CBufferObjPool& bfPool)
	: __super(hp, bfPool)
	{

	}

	void Reset(CONNID dwConnID, SOCKET soClient)
	{
		__super::Reset(dwConnID, soClient);

		host.Empty();
	}

	BOOL GetRemoteHost(LPCSTR* lpszHost, USHORT* pusPort = nullptr)
	{
		*lpszHost = host;

		if(pusPort)
			*pusPort = remoteAddr.Port();

		return (!host.IsEmpty());
	}
};

/* UDP ????????????????????? */
struct TUdpSocketObj : public TSocketObjBase
{
	using __super		= TSocketObjBase;
	using CRecvQueue	= CCASQueue<TItem>;

	PVOID				pHolder;
	FD					fdTimer;

	CBufferObjPool&		itPool;

	CRWLock				lcIo;
	CRWLock				lcSend;
	CCriSec				csSend;

	TBufferObjList		sndBuff;
	CRecvQueue			recvQueue;

	volatile DWORD		detectFails;

	static TUdpSocketObj* Construct(CPrivateHeap& hp, CBufferObjPool& bfPool)
	{
		TUdpSocketObj* pSocketObj = (TUdpSocketObj*)hp.Alloc(sizeof(TUdpSocketObj));
		ASSERT(pSocketObj);

		return new (pSocketObj) TUdpSocketObj(hp, bfPool);
	}

	static void Destruct(TUdpSocketObj* pSocketObj)
	{
		ASSERT(pSocketObj);

		CPrivateHeap& heap = pSocketObj->heap;
		pSocketObj->TUdpSocketObj::~TUdpSocketObj();
		heap.Free(pSocketObj);
	}
	
	TUdpSocketObj(CPrivateHeap& hp, CBufferObjPool& bfPool)
	: __super(hp), sndBuff(bfPool), itPool(bfPool)
	{

	}

	~TUdpSocketObj()
	{
		ClearRecvQueue();
	}

	static void Release(TUdpSocketObj* pSocketObj)
	{
		__super::Release(pSocketObj);

		pSocketObj->ClearRecvQueue();
		pSocketObj->sndBuff.Release();
	}

	int Pending()		{return sndBuff.Length();}
	BOOL IsPending()	{return Pending() > 0;}
	BOOL HasRecvData()	{return !recvQueue.IsEmpty();}

	static BOOL InvalidSocketObj(TUdpSocketObj* pSocketObj)
	{
		BOOL bDone = FALSE;

		if(TSocketObjBase::IsValid(pSocketObj))
		{
			pSocketObj->SetConnected(FALSE);

			CReentrantWriteLock	 locallock(pSocketObj->lcIo);
			CReentrantWriteLock	 locallock2(pSocketObj->lcSend);
			CCriSecLock			 locallock3(pSocketObj->csSend);

			if(TSocketObjBase::IsValid(pSocketObj))
			{
				TSocketObjBase::Invalid(pSocketObj);
				bDone = TRUE;
			}
		}

		return bDone;
	}

	void Reset(CONNID dwConnID)
	{
		__super::Reset(dwConnID);

		pHolder		= nullptr;
		fdTimer		= INVALID_FD;
		detectFails = 0;
	}

	void ClearRecvQueue()
	{
		TItem* pItem = nullptr;

		while(recvQueue.PopFront(&pItem))
			itPool.PutFreeItem(pItem);

		VERIFY(recvQueue.IsEmpty());
	}
};

/* ?????? TSocketObj ?????? */
typedef CRingCache2<TSocketObj, CONNID, true>		TSocketObjPtrPool;
/* ?????? TSocketObj ?????? */
typedef CRingPool<TSocketObj>						TSocketObjPtrList;
/* ?????? TSocketObj ???????????????????????? */
typedef CCASQueue<TSocketObj>						TSocketObjPtrQueue;

/* ?????? TSocketObj ?????? */
typedef CRingCache2<TAgentSocketObj, CONNID, true>	TAgentSocketObjPtrPool;
/* ?????? TSocketObj ?????? */
typedef CRingPool<TAgentSocketObj>					TAgentSocketObjPtrList;
/* ?????? TSocketObj ???????????????????????? */
typedef CCASQueue<TAgentSocketObj>					TAgentSocketObjPtrQueue;

/* ?????? TUdpSocketObj ?????? */
typedef CRingCache2<TUdpSocketObj, CONNID, true>	TUdpSocketObjPtrPool;
/* ?????? TUdpSocketObj ?????? */
typedef CRingPool<TUdpSocketObj>					TUdpSocketObjPtrList;
/* ?????? TUdpSocketObj ???????????????????????? */
typedef CCASQueue<TUdpSocketObj>					TUdpSocketObjPtrQueue;

/* HP_SOCKADDR ????????? */
struct hp_sockaddr_func
{
	struct hash
	{
		size_t operator() (const HP_SOCKADDR* pA) const
		{
			return pA->Hash();
		}
	};

	struct equal_to
	{
		bool operator () (const HP_SOCKADDR* pA, const HP_SOCKADDR* pB) const
		{
			return pA->EqualTo(*pB);
		}
	};

};

/* ??????-?????? ID ????????? */
typedef unordered_map<const HP_SOCKADDR*, CONNID, hp_sockaddr_func::hash, hp_sockaddr_func::equal_to>
										TSockAddrMap;
/* ??????-?????? ID ?????????????????? */
typedef TSockAddrMap::iterator			TSockAddrMapI;
/* ??????-?????? ID ????????? const ????????? */
typedef TSockAddrMap::const_iterator	TSockAddrMapCI;

/* IClient ????????????????????? */
struct TClientCloseContext
{
	BOOL bFireOnClose;
	EnSocketOperation enOperation;
	int iErrorCode;
	BOOL bNotify;

	TClientCloseContext(BOOL bFire = TRUE, EnSocketOperation enOp = SO_CLOSE, int iCode = SE_OK, BOOL bNtf = TRUE)
	{
		Reset(bFire, enOp, iCode, bNtf);
	}

	void Reset(BOOL bFire = TRUE, EnSocketOperation enOp = SO_CLOSE, int iCode = SE_OK, BOOL bNtf = TRUE)
	{
		bFireOnClose = bFire;
		enOperation	 = enOp;
		iErrorCode	 = iCode;
		bNotify		 = bNtf;
	}

};

/*****************************************************************************************************/
/******************************************** ?????????????????? ********************************************/
/*****************************************************************************************************/

/* ???????????????????????? */
LPCTSTR GetSocketErrorDesc(EnSocketError enCode);
/* ??????????????? */
ADDRESS_FAMILY DetermineAddrFamily(LPCTSTR lpszAddress);
/* ?????????????????????????????? HP_ADDR */
BOOL GetInAddr(LPCTSTR lpszAddress, HP_ADDR& addr);
/* ?????????????????????????????? HP_SOCKADDR */
BOOL GetSockAddr(LPCTSTR lpszAddress, USHORT usPort, HP_SOCKADDR& addr);
/* ??????????????????????????? IP ???????????? */
BOOL IsIPAddress(LPCTSTR lpszAddress, EnIPAddrType* penType = nullptr);
/* ????????????????????? IP ?????? */
BOOL GetIPAddress(LPCTSTR lpszHost, LPTSTR lpszIP, int& iIPLenth, EnIPAddrType& enType);
/* ????????????????????? HP_SOCKADDR */
BOOL GetSockAddrByHostName(LPCTSTR lpszHost, USHORT usPort, HP_SOCKADDR& addr);
/* ????????????????????? HP_SOCKADDR */
BOOL GetSockAddrByHostNameDirectly(LPCTSTR lpszHost, USHORT usPort, HP_SOCKADDR &addr);
/* ???????????? IP ?????? */
BOOL EnumHostIPAddresses(LPCTSTR lpszHost, EnIPAddrType enType, LPTIPAddr** lpppIPAddr, int& iIPAddrCount);
/* ?????? LPTIPAddr* */
BOOL RetrieveSockAddrIPAddresses(const vector<HP_PSOCKADDR>& vt, LPTIPAddr** lpppIPAddr, int& iIPAddrCount);
/* ?????? LPTIPAddr* */
BOOL FreeHostIPAddresses(LPTIPAddr* lppIPAddr);
/* ??? HP_SOCKADDR ?????????????????????????????? */
BOOL sockaddr_IN_2_A(const HP_SOCKADDR& addr, ADDRESS_FAMILY& usFamily, LPTSTR lpszAddress, int& iAddressLen, USHORT& usPort);
/* ??????????????????????????? HP_SOCKADDR ?????? */
BOOL sockaddr_A_2_IN(LPCTSTR lpszAddress, USHORT usPort, HP_SOCKADDR& addr);
/* ?????? Socket ?????????????????????????????? */
BOOL GetSocketAddress(SOCKET socket, LPTSTR lpszAddress, int& iAddressLen, USHORT& usPort, BOOL bLocal = TRUE);
/* ?????? Socket ????????????????????? */
BOOL GetSocketLocalAddress(SOCKET socket, LPTSTR lpszAddress, int& iAddressLen, USHORT& usPort);
/* ?????? Socket ????????????????????? */
BOOL GetSocketRemoteAddress(SOCKET socket, LPTSTR lpszAddress, int& iAddressLen, USHORT& usPort);
/* ?????????????????? */
BOOL SetMultiCastSocketOptions(SOCKET sock, const HP_SOCKADDR& bindAddr, const HP_SOCKADDR& castAddr, int iMCTtl, BOOL bMCLoop);

/* 64 ???????????????????????????????????? */
ULONGLONG NToH64(ULONGLONG value);
/* 64 ???????????????????????????????????? */
ULONGLONG HToN64(ULONGLONG value);

/* ??????????????????????????? */
#define ENDIAN_SWAP_16(A)	((USHORT)((((USHORT)(A) & 0xff00) >> 8) | (((USHORT)(A) & 0x00ff) << 8)))
/* ??????????????????????????? */
#define ENDIAN_SWAP_32(A)	((((DWORD)(A) & 0xff000000) >> 24) | \
							(((DWORD)(A) & 0x00ff0000) >>  8)  | \
							(((DWORD)(A) & 0x0000ff00) <<  8)  | \
							(((DWORD)(A) & 0x000000ff) << 24)	 )

/* ??????????????????????????? */
BOOL IsLittleEndian();
/* ?????????????????????????????????????????? */
USHORT HToLE16(USHORT value);
/* ?????????????????????????????????????????? */
USHORT HToBE16(USHORT value);
/* ?????????????????????????????????????????? */
DWORD HToLE32(DWORD value);
/* ?????????????????????????????????????????? */
DWORD HToBE32(DWORD value);

HRESULT ReadSmallFile(LPCTSTR lpszFileName, CFile& file, CFileMapping& fmap, DWORD dwMaxFileSize = MAX_SMALL_FILE_SIZE);
HRESULT MakeSmallFilePackage(LPCTSTR lpszFileName, CFile& file, CFileMapping& fmap, WSABUF szBuf[3], const LPWSABUF pHead = nullptr, const LPWSABUF pTail = nullptr);

/************************************************************************
?????????setsockopt() ????????????
???????????????????????? setsockopt() ??????
************************************************************************/

int SSO_SetSocketOption		(SOCKET sock, int level, int name, LPVOID val, int len);
int SSO_GetSocketOption		(SOCKET sock, int level, int name, LPVOID val, int* len);
int SSO_IoctlSocket			(SOCKET sock, long cmd, PVOID arg);

int SSO_NoBlock				(SOCKET sock, BOOL bNoBlock = TRUE);
int SSO_NoDelay				(SOCKET sock, BOOL bNoDelay = TRUE);
int SSO_DontLinger			(SOCKET sock, BOOL bDont = TRUE);
int SSO_Linger				(SOCKET sock, int l_onoff, int l_linger);
int SSO_KeepAlive			(SOCKET sock, BOOL bKeepAlive = TRUE);
int SSO_KeepAliveVals		(SOCKET sock, BOOL bOnOff, DWORD dwIdle, DWORD dwInterval, DWORD dwCount = 5);
int SSO_ReuseAddress		(SOCKET sock, EnReuseAddressPolicy opt);
int SSO_RecvBuffSize		(SOCKET sock, int size);
int SSO_SendBuffSize		(SOCKET sock, int size);
int SSO_RecvTimeOut			(SOCKET sock, int ms);
int SSO_SendTimeOut			(SOCKET sock, int ms);
int SSO_GetError			(SOCKET sock);

/* ?????? Connection ID */
CONNID GenerateConnectionID();
/* ?????? UDP ?????????????????? */
int IsUdpCloseNotify(const BYTE* pData, int iLength);
/* ?????? UDP ?????????????????? */
int SendUdpCloseNotify(SOCKET sock);
/* ?????? UDP ?????????????????? */
int SendUdpCloseNotify(SOCKET sock, const HP_SOCKADDR& remoteAddr);
/* ?????? Socket */
int ManualCloseSocket(SOCKET sock, int iShutdownFlag = 0xFF, BOOL bGraceful = TRUE);

#ifdef _ICONV_SUPPORT

#define CHARSET_GBK			"GBK"
#define CHARSET_UTF_8		"UTF-8"
#define CHARSET_UTF_16LE	"UTF-16LE"
#define CHARSET_UTF_32LE	"UTF-32LE"

// Charset A -> Charset B
BOOL CharsetConvert(LPCSTR lpszFromCharset, LPCSTR lpszToCharset, LPCSTR lpszInBuf, int iInBufLen, LPSTR lpszOutBuf, int& iOutBufLen);

// GBK -> UNICODE
BOOL GbkToUnicode(const char szSrc[], WCHAR szDest[], int& iDestLength);
// UNICODE -> GBK
BOOL UnicodeToGbk(const WCHAR szSrc[], char szDest[], int& iDestLength);
// UTF8 -> UNICODE
BOOL Utf8ToUnicode(const char szSrc[], WCHAR szDest[], int& iDestLength);
// UNICODE -> UTF8
BOOL UnicodeToUtf8(const WCHAR szSrc[], char szDest[], int& iDestLength);
// GBK -> UTF8
BOOL GbkToUtf8(const char szSrc[], char szDest[], int& iDestLength);
// UTF8 -> GBK
BOOL Utf8ToGbk(const char szSrc[], char szDest[], int& iDestLength);

#endif

// ?????? Base64 ???????????????
DWORD GuessBase64EncodeBound(DWORD dwSrcLen);
// ?????? Base64 ???????????????
DWORD GuessBase64DecodeBound(const BYTE* lpszSrc, DWORD dwSrcLen);
// Base64 ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int Base64Encode(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// Base64 ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int Base64Decode(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);

// ?????? URL ???????????????
DWORD GuessUrlEncodeBound(const BYTE* lpszSrc, DWORD dwSrcLen);
// ?????? URL ???????????????
DWORD GuessUrlDecodeBound(const BYTE* lpszSrc, DWORD dwSrcLen);
// URL ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int UrlEncode(BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// URL ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int UrlDecode(BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);

#ifdef _ZLIB_SUPPORT

// ???????????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int Compress(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// ???????????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int CompressEx(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen, int iLevel = Z_DEFAULT_COMPRESSION, int iMethod = Z_DEFLATED, int iWindowBits = MAX_WBITS, int iMemLevel = MAX_MEM_LEVEL, int iStrategy = Z_DEFAULT_STRATEGY);
// ???????????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int Uncompress(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// ???????????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int UncompressEx(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen, int iWindowBits = MAX_WBITS);
// ????????????????????????
DWORD GuessCompressBound(DWORD dwSrcLen, BOOL bGZip = FALSE);

// Gzip ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int GZipCompress(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// Gzip ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int GZipUncompress(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// ?????? Gzip ????????????????????????????????? 0 ??????????????????????????????????????????????????? Gzip ?????????
DWORD GZipGuessUncompressBound(const BYTE* lpszSrc, DWORD dwSrcLen);

#endif

#ifdef _BROTLI_SUPPORT

// Brotli ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int BrotliCompress(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// Brotli ???????????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int BrotliCompressEx(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen, int iQuality = BROTLI_DEFAULT_QUALITY, int iWindow = BROTLI_DEFAULT_WINDOW, BrotliEncoderMode enMode = BROTLI_DEFAULT_MODE);
// Brotli ?????????????????????0 -> ?????????-3 -> ????????????????????????-5 -> ????????????????????????
int BrotliUncompress(const BYTE* lpszSrc, DWORD dwSrcLen, BYTE* lpszDest, DWORD& dwDestLen);
// Brotli ????????????????????????
DWORD BrotliGuessCompressBound(DWORD dwSrcLen);

#endif
