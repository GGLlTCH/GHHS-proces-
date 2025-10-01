#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QObject>
#include <windows.h>

class SystemMonitor : public QObject
{
    Q_OBJECT

public:
    SystemMonitor(QObject *parent = nullptr);
    double getCpuUsage();
    double getRamUsage();
    double getDiskUsage();

private:
    ULONGLONG lastTotalTicks;
    ULONGLONG lastIdleTicks;
    bool firstCpuMeasure;
};

#endif // SYSTEMMONITOR_H