#include <ntddk.h>
#include <wdm.h>
#include "serialize.h"

static HANDLE hPipe = NULL;

VOID WritePipeMessage(LPCSTR szMessage, SIZE_T ullMessageSize)
{
	if (hPipe)
	{
		IO_STATUS_BLOCK IoStatusBlock;
		ZwWriteFile(hPipe, 0, NULL, NULL, &IoStatusBlock, (PVOID)szMessage, ullMessageSize, NULL, NULL);
	}
}

VOID HandlePipeMessage()
{
	if (hPipe)
	{
		IO_STATUS_BLOCK IoStatusBlock;
		CHAR PacketBuf[PACKET_SIZE_MAX];
		SIZE_T ullResponseSize;
		PCHAR ResponseBuf;
		LPSTR* ArgBuf;
		LPSTR szMessage;

		ZwReadFile(hPipe, NULL, NULL, NULL, &IoStatusBlock, PacketBuf, PACKET_SIZE_MAX, NULL, NULL);
		REQUEST_PACKET RequestPacket = { 0 };
		DeserializeRequest(PacketBuf, &RequestPacket);

		switch (RequestPacket.uOpcode)
		{
			case PACKET_REVERSE_ARG_ORDER:
				ullResponseSize = GetPacketSize(RequestPacket.uArgCount, RequestPacket.aullArgSizes);
				ResponseBuf = (PCHAR)MemAlloc(ullResponseSize);
				ArgBuf = (LPSTR*)MemAlloc(RequestPacket.uArgCount);
				if (ArgBuf && ResponseBuf)
				{
					for (INT i = (INT)(RequestPacket.uArgCount - 1); i >= 0; i--)
						ArgBuf[(RequestPacket.uArgCount - 1) - i] = (RequestPacket.ppszArgs)[i];

					SerializeRequest(PACKET_REVERSE_ARG_ORDER, RequestPacket.uArgCount, RequestPacket.aullArgSizes, ArgBuf, ResponseBuf);

					WritePipeMessage(ResponseBuf, ullResponseSize);
					FreeRequest(RequestPacket);
					MemFree(ArgBuf);
					MemFree(ResponseBuf);
				}
				break;

			default:
				szMessage = "Hello from kernel!";
				WritePipeMessage(szMessage, strlen(szMessage) + 1);;
		}
	}
}

VOID OpenPipe()
{
	UNICODE_STRING usPipeName;
	RtlInitUnicodeString(&usPipeName, L"\\Device\\NamedPipe\\MyPipe");

	OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
	InitializeObjectAttributes(&ObjectAttributes, &usPipeName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS ntStatus = ZwCreateFile(&hPipe, FILE_WRITE_DATA | SYNCHRONIZE, &ObjectAttributes, &IoStatusBlock, 0, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("ZwCreateFile fail, Status: %ld\n", ntStatus);
	}
}

VOID ClosePipe()
{
	if (hPipe)
	{
		ZwClose(hPipe);
		hPipe = NULL;
	}
}

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	// Will never use or create driver object -  This will be manually mapped
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	OpenPipe();
	HandlePipeMessage();
	ClosePipe();

	return STATUS_SUCCESS;
}