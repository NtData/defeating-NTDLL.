
#include <windows.h>
#include <iostream>
#include <winnt.h>
#include <sddl.h>

typedef enum _ACCESS_MODE {
    AccessModeRead,
    AccessModeWrite,
    AccessModeExecute
} ACCESS_MODE;

typedef struct _AUDIT_PARAMS {
    HANDLE   ObjectHandle;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    ACCESS_MASK DesiredAccess;
    ACCESS_MODE AccessMode;
} AUDIT_PARAMS;

bool CheckAccessAndAudit(AUDIT_PARAMS* params) {
    NTSTATUS status;
    HANDLE tokenHandle = NULL;
    SECURITY_SUBJECT_CONTEXT subjectContext;
    OBJECT_TYPE_LIST objectTypeList[1];
    ACCESS_MASK grantedAccessMask = 0;
    ULONG resultListLength = 0;

    objectTypeList[0].Level = 0;
    objectTypeList[0].ObjectType = 0;

    status = NtAccessCheckByTypeResultListAndAuditAlarmByHandle(
        L"ObjectName",
        NULL,
        L"ObjectTypeName",
        params->SecurityDescriptor,
        objectTypeList,
        1,
        params->DesiredAccess,
        params->AccessMode == AccessModeRead ? ACCESS_READ : (params->AccessMode == AccessModeWrite ? ACCESS_WRITE : ACCESS_EXECUTE),
        NULL,
        &resultListLength,
        &grantedAccessMask,
        &status,
        &subjectContext
    );

    if (NT_SUCCESS(status)) {
        std::wcout << L"[+] access check success: " << grantedAccessMask << std::endl;
        return true;
    } else {
        std::wcerr << L"[-] access chedk failed :(: " << status << std::endl;
        return false;
    }
}

int main() {
    PSECURITY_DESCRIPTOR securityDescriptor = NULL;
    ConvertStringSecurityDescriptorToSecurityDescriptor(L"D:(A;;FA;;;SY)(A;;FA;;;BA)", SDDL_REVISION_1, &securityDescriptor, NULL);

    AUDIT_PARAMS auditParams;
    auditParams.ObjectHandle = NULL;
    auditParams.SecurityDescriptor = securityDescriptor;
    auditParams.DesiredAccess = GENERIC_READ | GENERIC_WRITE;
    auditParams.AccessMode = AccessModeRead;

    if (CheckAccessAndAudit(&auditParams)) {
        std::wcout << L"[+] access chek completed [SUCCESS]" << std::endl;
    } else {
        std::wcerr << L"[-] access check encountered an error :(" << std::endl;
    }

    if (securityDescriptor) {
        LocalFree(securityDescriptor);
    }

    return 0;
}
