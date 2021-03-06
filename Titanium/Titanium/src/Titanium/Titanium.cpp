#include "Titanium.h"
#include <sstream>

#define TITANIUM_MEMORY_READ_REQUEST_32BIT					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x4554, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define TITANIUM_MEMORY_WRITE_REQUEST_32BIT					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x4584, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define TITANIUM_MEMORY_READ_REQUEST_64BIT					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7554, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define TITANIUM_MEMORY_WRITE_REQUEST_64BIT					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7584, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define TITANIUM_GET_TARGET_IMAGE_INFO_REQUEST_32BIT		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x4310, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define TITANIUM_GET_TARGET_IMAGE_INFO_REQUEST_64BIT		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7310, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define TITANIUM_INJECT_X64_DLL_REQUEST_64BIT				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1923, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define TITANIUM_INJECT_X86_DLL_REQUEST_64BIT				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1925, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#ifdef TITANIUM_X86
typedef struct _TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST
{
	ULONG ProcessID;
	ULONG pSource;
	ULONG pTarget;
	ULONG Size;
} TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST, *PTITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST;

typedef struct _TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST
{
	ULONG	pProcessName;
	ULONG	pTargetImageInfo;
} TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST, *PTITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST;
#endif // TITANIUM_X86

#ifdef TITANIUM_X64
typedef struct _TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST
{
	ULONG64 ProcessID;
	ULONG64 pSource;
	ULONG64 pTarget;
	ULONG64 Size;
} TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST, *PTITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST;

typedef struct _TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST
{
	ULONG64 pProcessName;
	ULONG64	pTargetImageInfo;
} TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST, *PTITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST;

typedef struct _TITANIUM_KERNEL_INJECT_DLL_REQUEST
{
	ULONG	ProcessID;
	ULONG64 pDLLPathBuffer;
	ULONG64 pBaseAddress;
} TITANIUM_KERNEL_INJECT_DLL_REQUEST, *PTITANIUM_KERNEL_INJECT_DLL_REQUEST;
#endif // TITANIUM_X64

TitaniumInterface::TitaniumInterface()
{
	hDriver = CreateFileA("\\\\.\\titanium", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
}

TitaniumInterface::~TitaniumInterface()
{
	CloseHandle(hDriver);
}

#ifdef TITANIUM_X86
void TitaniumInterface::ReadVirtualMemory(ULONG ProcessID, ULONG SourceAddr, void* TargetAddr, ULONG Size)
{
	TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST req;
	req.ProcessID = ProcessID;
	req.pSource = SourceAddr;
	req.pTarget = (ULONG)TargetAddr;
	req.Size = Size;

	DeviceIoControl(hDriver, TITANIUM_MEMORY_READ_REQUEST_32BIT, &req, sizeof(TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST), 0, 0, 0, 0);
}

void TitaniumInterface::WriteVirtualMemory(ULONG ProcessID, void* SourceAddr, ULONG TargetAddr, ULONG Size)
{
	TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST req;
	req.ProcessID = ProcessID;
	req.pSource = (ULONG)SourceAddr;
	req.pTarget = TargetAddr;
	req.Size = Size;

	DeviceIoControl(hDriver, TITANIUM_MEMORY_WRITE_REQUEST_32BIT, &req, sizeof(TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST), 0, 0, 0, 0);
}

TitaniumTargetImageInfo TitaniumInterface::GetTargetImageInfo(const wchar_t* ProcessName)
{
	TitaniumTargetImageInfo info;

	TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST req;
	req.pProcessName = (ULONG)ProcessName;
	req.pTargetImageInfo = (ULONG)&info;

	DeviceIoControl(hDriver, TITANIUM_GET_TARGET_IMAGE_INFO_REQUEST_32BIT, &req, sizeof(TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST), 0, 0, 0, 0);

	return info;
}

ULONG TitaniumMemory::PatternScan(ULONG pid, ULONG start, ULONG size, const std::string& pattern)
{
	std::string sig, mask;
	CreateSignatureAndMask(pattern, sig, mask);

	BYTE* data = new BYTE[size];
	iface.ReadVirtualMemory(pid, start, data, size);

	for (DWORD i = 0; i < size; i++)
		if (CompareMemory((const BYTE*)(data + i), (const BYTE*)sig.c_str(), mask.c_str()))
			return start + i;

	delete[] data;
	return NULL;
}
#endif // TITANIUM_X86

#ifdef TITANIUM_X64
void TitaniumInterface::ReadVirtualMemory(ULONG64 ProcessID, ULONG64 SourceAddr, void* TargetAddr, ULONG64 Size)
{
	TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST req;
	req.ProcessID = ProcessID;
	req.pSource = SourceAddr;
	req.pTarget = (ULONG64)TargetAddr;
	req.Size = Size;

	DeviceIoControl(hDriver, TITANIUM_MEMORY_READ_REQUEST_64BIT, &req, sizeof(TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST), 0, 0, 0, 0);
}

void TitaniumInterface::WriteVirtualMemory(ULONG64 ProcessID, void* SourceAddr, ULONG64 TargetAddr, ULONG64 Size)
{
	TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST req;
	req.ProcessID = ProcessID;
	req.pSource = (ULONG64)SourceAddr;
	req.pTarget = TargetAddr;
	req.Size = Size;

	DeviceIoControl(hDriver, TITANIUM_MEMORY_WRITE_REQUEST_64BIT, &req, sizeof(TITANIUM_KERNEL_MEMORY_READ_WRITE_REQUEST), 0, 0, 0, 0);
}

TitaniumTargetImageInfo TitaniumInterface::GetTargetImageInfo(const wchar_t* ProcessName)
{
	TitaniumTargetImageInfo info;

	TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST req;
	req.pProcessName = (ULONG64)ProcessName;
	req.pTargetImageInfo = (ULONG64)&info;

	DeviceIoControl(hDriver, TITANIUM_GET_TARGET_IMAGE_INFO_REQUEST_64BIT, &req, sizeof(TITANIUM_KERNEL_GET_TARGET_IMAGE_INFO_REQUEST), 0, 0, 0, 0);

	return info;
}

ULONG64 TitaniumInterface::InjectX64DLL(ULONG ProcessID, const wchar_t* DllPath)
{
	ULONG64 BaseAddress = 0;

	TITANIUM_KERNEL_INJECT_DLL_REQUEST req;
	req.ProcessID = ProcessID;
	req.pDLLPathBuffer = (ULONG64)DllPath;
	req.pBaseAddress = (ULONG64)&BaseAddress;

	DeviceIoControl(hDriver, TITANIUM_INJECT_X64_DLL_REQUEST_64BIT, &req, sizeof(TITANIUM_KERNEL_INJECT_DLL_REQUEST), 0, 0, 0, 0);

	return BaseAddress;
}

ULONG64 TitaniumInterface::InjectX86DLL(ULONG ProcessID, const wchar_t* DllPath)
{
	ULONG64 BaseAddress = 0;

	TITANIUM_KERNEL_INJECT_DLL_REQUEST req;
	req.ProcessID = ProcessID;
	req.pDLLPathBuffer = (ULONG64)DllPath;
	req.pBaseAddress = (ULONG64)&BaseAddress;

	DeviceIoControl(hDriver, TITANIUM_INJECT_X86_DLL_REQUEST_64BIT, &req, sizeof(TITANIUM_KERNEL_INJECT_DLL_REQUEST), 0, 0, 0, 0);

	return BaseAddress;
}

ULONG64 TitaniumMemory::PatternScan(ULONG pid, ULONG64 start, ULONG64 size, const std::string& pattern)
{
	std::string sig, mask;
	CreateSignatureAndMask(pattern, sig, mask);

	BYTE* data = new BYTE[size];
	iface.ReadVirtualMemory(pid, start, data, size);

	for (ULONG64 i = 0; i < size; i++)
		if (CompareMemory((const BYTE*)(data + i), (const BYTE*)sig.c_str(), mask.c_str()))
			return start + i;

	delete[] data;
	return NULL;
}
#endif // TITANIUM_X64

bool TitaniumMemory::CompareMemory(const BYTE* bData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++bData, ++bMask)
		if (*szMask == 'x' && *bData != *bMask)
			return false;

	return (*szMask == NULL);
}

void TitaniumMemory::CreateSignatureAndMask(const std::string& pattern, std::string& signature, std::string& mask)
{
	auto IsHex = [](char c) -> bool {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	};

	char buffer[2];
	std::stringstream sig_s;
	std::stringstream mask_s;

	for (size_t i = 0, l = pattern.size() - 1; i < l; i++)
	{
		if (IsHex(pattern[i]))
		{
			buffer[0] = pattern[i];
			buffer[1] = (l >= i + 1 && IsHex(pattern[i + 1])) ? pattern[++i] : 0;
			sig_s << (char)strtol(buffer, nullptr, 16);
			mask_s << 'x';
			continue;
		}
		else if (pattern[i] == '?' || pattern[i] == '*')
		{
			sig_s << "\x90";
			mask_s << '?';
			continue;
		}
	}

	signature = sig_s.str();
	mask = mask_s.str();
}
