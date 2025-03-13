++
#inlude <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <locale>       // Для локализации
#include <codecvt>      // Для преобразования строк wstring <-> string

// Функция преобразования wstring в string (используется для локализации)
std::string wstring_to_string(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Функция преобразования string в wstring (используется для локализации)
std::wstring string_to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

struct ProcessInfo {
    DWORD pid;
    std::wstring name;
    std::wstring path; // Добавлено: путь к исполняемому файлу
};

// Функция для получения списка процессов
std::vector<ProcessInfo> GetProcessList() {
    std::vector<ProcessInfo> processList;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << wstring_to_string(L"Ошибка: CreateToolhelp32Snapshot не удалось") << std::endl;
        return processList;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << wstring_to_string(L"Ошибка: Process32First не удалось") << std::endl;
        CloseHandle(hProcessSnap);
        return processList;
    }

    do {
        ProcessInfo pi;
        pi.pid = pe32.th32ProcessID;
        pi.name = pe32.szExeFile;

        // Получаем путь к исполняемому файлу
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pi.pid);
        if (hProcess != NULL) {
            wchar_t szFilename[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, NULL, szFilename, MAX_PATH)) {
                pi.path = szFilename;
            } else {
                pi.path = L"N/A";
            }
            CloseHandle(hProcess);
        } else {
            pi.path = L"N/A";
        }

        processList.push_back(pi);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return processList;
}

// Функция для завершения процесса
bool KillProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << wstring_to_string(L"Ошибка: OpenProcess не удалось: ") << GetLastError() << std::endl;
        return false;
    }

    if (!TerminateProcess(hProcess, 1)) {
        std::cerr << wstring_to_string(L"Ошибка: TerminateProcess не удалось: ") << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

// Функция для приостановки процесса
bool SuspendProcess(DWORD pid) {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        std::cerr << wstring_to_string(L"Ошибка: CreateToolhelp32Snapshot (потоки) не удалось") << std::endl;
        return false;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32)) {
        std::cerr << wstring_to_string(L"Ошибка: Thread32First не удалось") << std::endl;
        CloseHandle(hThreadSnap);
        return false;
    }

    do {
        if (te32.th32OwnerProcessID == pid) {
            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
            if (hThread != NULL) {
                if(SuspendThread(hThread) == (DWORD)-1){
                     std::cerr << wstring_to_string(L"Ошибка: SuspendThread не удалось: ") << GetLastError() << std::endl;
                }

                CloseHandle(hThread);
            }
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return true;
}

// Функция для возобновления процесса
bool ResumeProcess(DWORD pid) {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        std::cerr << wstring_to_string(L"Ошибка: CreateToolhelp32Snapshot (потоки) не удалось") << std::endl;
        return false;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32)) {
        std::cerr << wstring_to_string(L"Ошибка: Thread32First не удалось") << std::endl;
        CloseHandle(hThreadSnap);
        return false;
    }

    do {
        if (te32.th32OwnerProcessID == pid) {
            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
            if (hThread != NULL) {
                if(ResumeThread(hThread) == (DWORD)-1){
                     std::cerr << wstring_to_string(L"Ошибка: ResumeThread не удалось: ") << GetLastError() << std::endl;
                }
                CloseHandle(hThread);
            }
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return true;
}


// Функция для поиска процесса по полному пути
DWORD FindProcessByPath(const std::wstring& path) {
    std::vector<ProcessInfo> processes = GetProcessList();
    for (const auto& process : processes) {
        if (process.path == path) {
            return process.pid;
        }
    }
    return 0; // Возвращает 0, если процесс не найден
}

int main() {
    // Устанавливаем русскую локаль для консоли (работает не всегда идеально)
    std::locale::global(std::locale(""));
    SetConsoleOutputCP(1251); // Для Windows, кодировка CP1251

    std::wcout << L"Список процессов:" << std::endl;
    std::vector<ProcessInfo> processes = GetProcessList();
    for (const auto& process : processes) {
        std::wcout << L"PID: " << process.pid << L", Имя: " << process.name << L", Путь: " << process.path << std::endl;
    }

    DWORD pidToKill;
    std::cout << wstring_to_string(L"Введите PID процесса для завершения: ");
    std::cin >> pidToKill;

    if (KillProcess(pidToKill)) {
        std::cout << wstring_to_string(L"Процесс успешно завершен.") << std::endl;
    } else {
        std::cout << wstring_to_string(L"Не удалось завершить процесс.") << std::endl;
    }

    DWORD pidToSuspend;
    std::cout << wstring_to_string(L"Введите PID процесса для приостановки: ");
    std::cin >> pidToSuspend;

    if (SuspendProcess(pidToSuspend)) {
        std::cout << wstring_to_string(L"Процесс успешно приостановлен.") << std::endl;
    } else {
        std::cout << wstring_to_string(L"Не удалось приостановить процесс.") << std::endl;
    }


    DWORD pidToResume;
    std::cout << wstring_to_string(L"Введите PID процесса для возобновления: ");
    std::cin >> pidToResume;

    if (ResumeProcess(pidToResume)) {
        std::cout << wstring_to_string(L"Процесс успешно возобновлен.") << std::endl;
    } else {
        std::cout << wstring_to_string(L"Не удалось возобновить процесс.") << std::endl;
    }

    std::wstring pathToFind;
    std::cout << wstring_to_string(L"Введите полный путь к исполняемому файлу для поиска: ");
    std::getline(std::wcin >> std::ws, pathToFind); // Чтение строки с пробелами

    DWORD foundPid = FindProcessByPath(pathToFind);
    if (foundPid != 0) {
        std::wcout << L"Процесс найден, PID: " << foundPid << std::endl;
    } else {
        std::wcout << L"Процесс не найден." << std::endl;
    }


    return 0;
}
