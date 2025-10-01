#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

struct ProcessInfo {
    int pid;
    QString name;
    double cpuUsage;
    double memoryUsage;
    int threadCount;
    QString userName;
    QString status;
    QString exePath;
};

class ProcessManager : public QObject
{
    Q_OBJECT

public:
    ProcessManager(QObject *parent = nullptr);
    std::vector<ProcessInfo> getProcesses();
    int getProcessCount();
    bool killProcess(int pid);
    bool suspendProcess(int pid);
    bool resumeProcess(int pid);
    ProcessInfo getProcessDetails(int pid);

private:
    QString getUserNameFromPID(DWORD pid);
    QString getProcessPath(DWORD pid);
    double calculateCpuUsage(DWORD pid, ULONGLONG &lastTime, ULONGLONG &lastSystemTime);
    
    std::unordered_map<DWORD, std::pair<ULONGLONG, ULONGLONG>> processTimes;
};

#endif // PROCESSMANAGER_H