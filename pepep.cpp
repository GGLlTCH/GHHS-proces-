#include <iostream>
#include <windows.h>
#include <tlhelp32.h>  

#include <vector>
#include <string>

struct ProcessInfo {
    DWORD pid;
    std::wstring name;
    // Добавьте другие поля (использование CPU, памяти) позже
};

std::vector<ProcessInfo> GetProcessList() {
    std::vector<ProcessInfo> processList;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateToolhelp32Snapshot failed" << std::endl;
        return processList;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << "Process32First failed" << std::endl;
        CloseHandle(hProcessSnap);
        return processList;
    }

    do {
        ProcessInfo pi;
        pi.pid = pe32.th32ProcessID;
        pi.name = pe32.szExeFile;
        processList.push_back(pi);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return processList;
}


bool KillProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;
        return false;
    }

    if (!TerminateProcess(hProcess, 1)) {
        std::cerr << "TerminateProcess failed: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

int main() {
    std::vector<ProcessInfo> processes = GetProcessList();

    std::wcout << L"Processes:" << std::endl;
    for (const auto& process : processes) {
        std::wcout << L"PID: " << process.pid << L", Name: " << process.name << std::endl;
    }

    // Пример завершения процесса
    DWORD pidToKill;
    std::cout << "Enter PID to kill: ";
    std::cin >> pidToKill;

    if (KillProcess(pidToKill)) {
        std::cout << "Process killed successfully." << std::endl;
    } else {
        std::cout << "Failed to kill process." << std::endl;
    }

    return 0;
}
