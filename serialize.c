#include "serialize.h"

SIZE_T GetPacketSize(INT iArgCount, SIZE_T aullArgSizes[])
{
	SIZE_T ullTotalArgSize = 0;
	for (INT i = 0; i < iArgCount; i++)
		ullTotalArgSize += aullArgSizes[i];

	return ullTotalArgSize + (sizeof(UINT32) * 2) + (sizeof(SIZE_T) * iArgCount);
}

VOID SerializeRequest(UINT32 uOpcode, UINT32 uArgCount, SIZE_T aiArgSizes[], LPSTR aszArgList[], _Out_ PCHAR pRequestData)
{
	if (pRequestData)
	{
		*(UINT32*)pRequestData = uOpcode;
		*(UINT32*)(pRequestData + sizeof(UINT32)) = uArgCount;

		for (UINT32 i = 0; i < uArgCount; i++)
			// setting the argument sizes
			*(SIZE_T*)(pRequestData + (2 * sizeof(UINT32)) + (sizeof(SIZE_T) * i)) = aiArgSizes[i];

		SIZE_T ullOffset = (2 * sizeof(UINT32)) + (sizeof(SIZE_T) * uArgCount);
		for (UINT32 i = 0; i < uArgCount; i++)
		{
			RtlCopyMemory((PCHAR)(pRequestData + (int)ullOffset), aszArgList[i], aiArgSizes[i]);
			ullOffset += aiArgSizes[i];
		}
	}
}

VOID DeserializeRequest(PCHAR pRequestBuf, LPREQUEST_PACKET lpRequest)
{
	if (pRequestBuf && lpRequest)
	{
		lpRequest->uOpcode = *(UINT32*)(pRequestBuf);
		UINT32 uArgCount = *(UINT32*)(pRequestBuf + sizeof(UINT32));

		lpRequest->uArgCount = uArgCount;
		lpRequest->ppszArgs = (LPSTR*)MemAlloc(uArgCount * sizeof(LPSTR));

		SIZE_T ullOffset = (2 * sizeof(UINT32));

		PSIZE_T aullArgSizes = (PSIZE_T)MemAlloc(uArgCount * sizeof(SIZE_T));

		if (aullArgSizes)
		{
			for (UINT32 i = 0; i < uArgCount; i++)
			{
				aullArgSizes[i] = *(SIZE_T*)(pRequestBuf + ullOffset);
				ullOffset += sizeof(SIZE_T);
			}
			for (UINT32 i = 0; i < uArgCount; i++)
			{
				LPSTR szArg = (LPSTR)MemAlloc(aullArgSizes[i]);
				if (szArg)
				{
					RtlCopyMemory(szArg, (PCHAR)(pRequestBuf + (int)ullOffset), aullArgSizes[i]);
					if (lpRequest->ppszArgs)
						(lpRequest->ppszArgs)[i] = szArg;

					ullOffset += aullArgSizes[i];
				}
			}
			lpRequest->aullArgSizes = aullArgSizes;
		}
	}
}

VOID FreeRequest(REQUEST_PACKET Request)
{
	if (Request.ppszArgs)
	{
		for (UINT32 i = 0; i < Request.uArgCount; i++)
		{
			MemFree((Request.ppszArgs)[i]);
		}
		MemFree(Request.ppszArgs);
	}
	if (Request.aullArgSizes)
		MemFree(Request.aullArgSizes);
}