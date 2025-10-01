#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QTabWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include "ProcessManager.h"
#include "SystemMonitor.h"

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateData();
    void onProcessSelected(QTreeWidgetItem *item, int column);
    void onKillProcess();
    void onSuspendProcess();
    void onResumeProcess();
    void onSearchChanged(const QString &text);
    void onAutoUpdateToggled(bool checked);

private:
    void setupUI();
    void setupCharts();
    void updateProcessList();
    void updateSystemInfo();
    void updateCharts();
    
    QTimer *updateTimer;
    ProcessManager *processManager;
    SystemMonitor *systemMonitor;
    
    // UI элементы
    QTabWidget *tabWidget;
    QTreeWidget *processTree;
    QLineEdit *searchEdit;
    QCheckBox *autoUpdateCheck;
    
    // Системная информация
    QLabel *cpuLabel;
    QLabel *ramLabel;
    QLabel *diskLabel;
    QLabel *processCountLabel;
    
    // Графики
    QChart *cpuChart;
    QChart *ramChart;
    QChart *diskChart;
    QLineSeries *cpuSeries;
    QLineSeries *ramSeries;
    QLineSeries *diskSeries;
    QChartView *cpuChartView;
    QChartView *ramChartView;
    QChartView *diskChartView;
    
    // Данные для графиков
    QVector<QPointF> cpuData;
    QVector<QPointF> ramData;
    QVector<QPointF> diskData;
    int timeCounter;
    
    bool autoUpdateEnabled;
};

#endif // MAINWINDOW_H