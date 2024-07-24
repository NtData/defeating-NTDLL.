#include <iostream>
#include <windows.h>

enum ProcessState {
    RUNNING,
    SUSPENDED,
    TERMINATED
};

typedef LONG (NTAPI *NtSuspendProcessFunc)(IN HANDLE ProcessHandle);
typedef LONG (NTAPI *NtResumeProcessFunc)(IN HANDLE ProcessHandle);

NtSuspendProcessFunc NtSuspendProcess;
NtResumeProcessFunc NtResumeProcess;

bool LoadNtFunctions() {
    HMODULE hNtDll = LoadLibraryA("ntdll.dll");
    if (!hNtDll) {
        std::cerr << "[ERRPR] failed to load ntdll.dll" << std::endl;
        return false;
    }

    NtSuspendProcess = (NtSuspendProcessFunc)GetProcAddress(hNtDll, "NtSuspendProcess");
    NtResumeProcess = (NtResumeProcessFunc)GetProcAddress(hNtDll, "NtResumeProcess");

    if (!NtSuspendProcess || !NtResumeProcess) {
        std::cerr << "[ERROR] failed to load NtSuspendProcess/NtResumeProcess" << std::endl;
        FreeLibrary(hNtDll);
        return false;
    }

    return true;
}

bool SuspendProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, processId);
    if (!hProcess) {
        std::cerr << "[+] failed to open process: " << GetLastError() << std::endl;
        return false;
    }

    if (NtSuspendProcess(hProcess) != 0) {
        std::cerr << "[+] unable to suspend process.: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

bool ResumeProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, processId);
    if (!hProcess) {
        std::cerr << "[+] failed to open process: " << GetLastError() << std::endl;
        return false;
    }

    if (NtResumeProcess(hProcess) != 0) {
        std::cerr << "[+] unable to resume process: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

bool TerminateProcessById(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (!hProcess) {
        std::cerr << "[+] failed to open thhe process: " << GetLastError() << std::endl;
        return false;
    }

    if (!TerminateProcess(hProcess, 0)) {
        std::cerr << "[+] unable to terminate process: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

bool NtChangeProcessState(DWORD processId, ProcessState newState) {
    switch (newState) {
        case RUNNING:
            return ResumeProcess(processId);
        case SUSPENDED:
            return SuspendProcess(processId);
        case TERMINATED:
            return TerminateProcessById(processId);
        default:
            std::cerr << "no idea tbh" << std::endl;
            return false;
    }
}

int main() {
    if (!LoadNtFunctions()) {
        return 1;
    }

    DWORD processId = 1948; 
    ProcessState newState = SUSPENDED;

    if (NtChangeProcessState(processId, newState)) {
        std::cout << "[SUCCESS] Process state changed" << std::endl;
    } else {
        std::cout << "[+] failed to change process state." << std::endl;
    }

    return 0;
}
