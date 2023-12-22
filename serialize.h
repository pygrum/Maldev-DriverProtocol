#pragma once
#ifdef _KERNEL_MODE
#include <wdm.h>
#define MemAlloc(size) ExAllocatePool2(POOL_FLAG_PAGED, size, 0x41414141)
#define MemFree(p) ExFreePool(p)
#pragma warning( disable : 4244 )
#pragma warning( disable : 4242 )
#else
#include <Windows.h>
#define MemAlloc(size) malloc(size)
#define MemFree(p) free(p)
#endif

#define PACKET_SIZE_MAX 512

#define PACKET_REVERSE_ARG_ORDER 0x1000

typedef struct _REQUEST_PACKET {
	UINT32 uOpcode;
	UINT32 uArgCount;
	PSIZE_T aullArgSizes;
	LPSTR* ppszArgs;

} REQUEST_PACKET, * LPREQUEST_PACKET;

VOID SerializeRequest(UINT32 uOpcode, UINT32 iArgCount, SIZE_T aiArgSizes[], LPSTR aszArgList[], _Out_ PCHAR pRequestData);

VOID DeserializeRequest(PCHAR pRequestBuf, LPREQUEST_PACKET lpRequest);

VOID FreeRequest(REQUEST_PACKET Request);

SIZE_T GetPacketSize(INT iArgCount, SIZE_T aiArgSizes[]);