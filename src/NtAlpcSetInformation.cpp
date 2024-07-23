/* unfinished i gave up */
#include <Windows.h>
#include <iostream>

#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWCH   Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _ALPC_PORT_ATTRIBUTES {
    ULONG Flags;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    ULONG MaxMessageLength;
    ULONG MemoryBandwidth;
    ULONG MaxPoolUsage;
    ULONG MaxSectionSize;
    ULONG MaxViewSize;
    ULONG MaxTotalSectionSize;
    ULONG DupObjectTypes;
    ULONG Reserved[2];
} ALPC_PORT_ATTRIBUTES;

extern "C" {
    NTSTATUS NTAPI NtCreatePort(
        PHANDLE PortHandle,
        PVOID PortAttributes
    );

    NTSTATUS NTAPI NtConnectPort(
        PHANDLE PortHandle,
        PUNICODE_STRING PortName,
        PVOID PortAttributes,
        PVOID MaxMessageLength,
        PVOID SecurityQos,
        PVOID ClientSecurityQos,
        PVOID ConnectionInformation,
        PVOID ReturnConnectionInformation
    );

    NTSTATUS NTAPI NtAlpcSetInformation(
        HANDLE PortHandle,
        ULONG PortInformationClass,
        PVOID PortInformation,
        ULONG Length
    );

    VOID NTAPI RtlInitUnicodeString(
        PUNICODE_STRING DestinationString,
        PCWSTR SourceString
    );
}

int main() {
    HMODULE hNtDll = LoadLibraryA("ntdll.dll");
    if (hNtDll == NULL) {
        std::cerr << "[ERROR ALERT] unable to load ntdll" << std::endl;
        return 1;
    }

    auto NtCreatePort = (decltype(&::NtCreatePort))GetProcAddress(hNtDll, "NtCreatePort");
    auto NtConnectPort = (decltype(&::NtConnectPort))GetProcAddress(hNtDll, "NtConnectPort");
    auto NtAlpcSetInformation = (decltype(&::NtAlpcSetInformation))GetProcAddress(hNtDll, "NtAlpcSetInformation");
    auto RtlInitUnicodeString = (decltype(&::RtlInitUnicodeString))GetProcAddress(hNtDll, "RtlInitUnicodeString");

    if (NtCreatePort == NULL || NtConnectPort == NULL || NtAlpcSetInformation == NULL || RtlInitUnicodeString == NULL) {
        std::cerr << "[ERROR ALERT] unable to get NT functions brov :sob" << std::endl;
        FreeLibrary(hNtDll);
        return 1;
    }

    HANDLE portHandle = NULL;
    ALPC_PORT_ATTRIBUTES portAttributes = {0};
    portAttributes.Flags = 0;
    portAttributes.MaxMessageLength = 256;
    portAttributes.SecurityQos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    portAttributes.SecurityQos.ImpersonationLevel = SecurityImpersonation;
    portAttributes.SecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    portAttributes.SecurityQos.EffectiveOnly = FALSE;

    NTSTATUS status = NtCreatePort(&portHandle, &portAttributes);
    if (status != 0 || portHandle == NULL) {
        std::cerr << "[ERROR ALERT] NtCreatePort failed with status: 0x" 
                  << std::hex << status << std::endl;
        FreeLibrary(hNtDll);
        return 1;
    }

    UNICODE_STRING portName;
    RtlInitUnicodeString(&portName, L"\\Test");

    HANDLE connectedPortHandle = NULL;
    status = NtConnectPort(&connectedPortHandle, &portName, NULL, NULL, NULL, NULL, NULL, NULL);
    if (status != 0 || connectedPortHandle == NULL) {
        std::cerr << "[ERROR ALERT] NtConnectPort failed with status 0x" 
                  << std::hex << status << std::endl;
        CloseHandle(portHandle);
        FreeLibrary(hNtDll);
        return 1;
    }

    ULONG portInfoClass = 0;
    ULONG length = sizeof(ALPC_PORT_ATTRIBUTES);

    status = NtAlpcSetInformation(connectedPortHandle, portInfoClass, &portAttributes, length);
    if (status != 0) {
        std::cerr << "[ERROR ALERT]NtAlpcSetInformation failed with status: 0x" 
                  << std::hex << status << std::endl;
    } else {
        std::cout << "[SUCCESS] NtAlpcSetInformation succeeded" << std::endl;
    }

    CloseHandle(connectedPortHandle);
    CloseHandle(portHandle);
    FreeLibrary(hNtDll);

    return 0;
}
