#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), timeCounter(0), autoUpdateEnabled(true)
{
    processManager = new ProcessManager(this);
    systemMonitor = new SystemMonitor(this);
    
    setupUI();
    setupCharts();
    
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateData);
    updateTimer->start(1000); // Обновление каждую секунду
    
    setWindowTitle("GHHS Process Manager");
    setMinimumSize(1400, 900);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Верхняя панель с системной информацией
    QHBoxLayout *infoLayout = new QHBoxLayout();
    
    cpuLabel = new QLabel("CPU: 0%");
    ramLabel = new QLabel("RAM: 0%");
    diskLabel = new QLabel("Disk: 0%");
    processCountLabel = new QLabel("Processes: 0");
    
    // Стилизация метрик
    QString labelStyle = "QLabel { font-weight: bold; font-size: 12px; padding: 5px; }";
    cpuLabel->setStyleSheet(labelStyle + "color: #00ccff;");
    ramLabel->setStyleSheet(labelStyle + "color: #ff5555;");
    diskLabel->setStyleSheet(labelStyle + "color: #55ff55;");
    processCountLabel->setStyleSheet(labelStyle + "color: #ffffff;");
    
    infoLayout->addWidget(cpuLabel);
    infoLayout->addWidget(ramLabel);
    infoLayout->addWidget(diskLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(processCountLabel);
    
    // Панель поиска и управления
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск процессов...");
    searchEdit->setMaximumWidth(300);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    
    autoUpdateCheck = new QCheckBox("Автообновление");
    autoUpdateCheck->setChecked(true);
    connect(autoUpdateCheck, &QCheckBox::toggled, this, &MainWindow::onAutoUpdateToggled);
    
    QPushButton *killBtn = new QPushButton("Завершить");
    QPushButton *refreshBtn = new QPushButton("Обновить");
    QPushButton *suspendBtn = new QPushButton("Приостановить");
    QPushButton *resumeBtn = new QPushButton("Возобновить");
    
    connect(killBtn, &QPushButton::clicked, this, &MainWindow::onKillProcess);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::updateProcessList);
    connect(suspendBtn, &QPushButton::clicked, this, &MainWindow::onSuspendProcess);
    connect(resumeBtn, &QPushButton::clicked, this, &MainWindow::onResumeProcess);
    
    controlLayout->addWidget(searchEdit);
    controlLayout->addWidget(autoUpdateCheck);
    controlLayout->addStretch();
    controlLayout->addWidget(refreshBtn);
    controlLayout->addWidget(suspendBtn);
    controlLayout->addWidget(resumeBtn);
    controlLayout->addWidget(killBtn);
    
    // Табы
    tabWidget = new QTabWidget();
    
    // Вкладка процессов
    QWidget *processTab = new QWidget();
    QVBoxLayout *processLayout = new QVBoxLayout(processTab);
    
    processTree = new QTreeWidget();
    processTree->setHeaderLabels({"PID", "Имя", "CPU%", "Память (MB)", "Потоки", "Пользователь", "Статус"});
    processTree->setSortingEnabled(true);
    processTree->setAlternatingRowColors(true);
    processTree->header()->setSectionResizeMode(QHeaderView::Interactive);
    processTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    connect(processTree, &QTreeWidget::itemClicked, this, &MainWindow::onProcessSelected);
    
    processLayout->addWidget(processTree);
    
    // Вкладка графиков
    QWidget *chartsTab = new QWidget();
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsTab);
    
    QHBoxLayout *chartsRow1 = new QHBoxLayout();
    chartsRow1->addWidget(cpuChartView);
    chartsRow1->addWidget(ramChartView);
    
    QHBoxLayout *chartsRow2 = new QHBoxLayout();
    chartsRow2->addWidget(diskChartView);
    
    chartsLayout->addLayout(chartsRow1);
    chartsLayout->addLayout(chartsRow2);
    
    tabWidget->addTab(processTab, "Процессы");
    tabWidget->addTab(chartsTab, "Графики нагрузки");
    
    mainLayout->addLayout(infoLayout);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(tabWidget);
}

void MainWindow::setupCharts()
{
    // CPU Chart
    cpuChart = new QChart();
    cpuSeries = new QLineSeries();
    cpuSeries->setName("CPU Usage");
    cpuSeries->setColor(QColor(0, 204, 255));
    
    cpuChart->addSeries(cpuSeries);
    cpuChart->setTitle("Загрузка CPU");
    cpuChart->setTheme(QChart::ChartThemeDark);
    cpuChart->createDefaultAxes();
    cpuChart->axisX()->setTitleText("Время (сек)");
    cpuChart->axisY()->setTitleText("%");
    cpuChart->axisY()->setRange(0, 100);
    
    cpuChartView = new QChartView(cpuChart);
    cpuChartView->setRenderHint(QPainter::Antialiasing);
    
    // RAM Chart
    ramChart = new QChart();
    ramSeries = new QLineSeries();
    ramSeries->setName("RAM Usage");
    ramSeries->setColor(QColor(255, 85, 85));
    
    ramChart->addSeries(ramSeries);
    ramChart->setTitle("Использование RAM");
    ramChart->setTheme(QChart::ChartThemeDark);
    ramChart->createDefaultAxes();
    ramChart->axisX()->setTitleText("Время (сек)");
    ramChart->axisY()->setTitleText("%");
    ramChart->axisY()->setRange(0, 100);
    
    ramChartView = new QChartView(ramChart);
    ramChartView->setRenderHint(QPainter::Antialiasing);
    
    // Disk Chart
    diskChart = new QChart();
    diskSeries = new QLineSeries();
    diskSeries->setName("Disk Usage");
    diskSeries->setColor(QColor(85, 255, 85));
    
    diskChart->addSeries(diskSeries);
    diskChart->setTitle("Загрузка диска");
    diskChart->setTheme(QChart::ChartThemeDark);
    diskChart->createDefaultAxes();
    diskChart->axisX()->setTitleText("Время (сек)");
    diskChart->axisY()->setTitleText("%");
    diskChart->axisY()->setRange(0, 100);
    
    diskChartView = new QChartView(diskChart);
    diskChartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::updateData()
{
    if (!autoUpdateEnabled) return;
    
    updateSystemInfo();
    updateProcessList();
    updateCharts();
}

void MainWindow::updateSystemInfo()
{
    double cpuUsage = systemMonitor->getCpuUsage();
    double ramUsage = systemMonitor->getRamUsage();
    double diskUsage = systemMonitor->getDiskUsage();
    int processCount = processManager->getProcessCount();
    
    cpuLabel->setText(QString("CPU: %1%").arg(cpuUsage, 0, 'f', 1));
    ramLabel->setText(QString("RAM: %1%").arg(ramUsage, 0, 'f', 1));
    diskLabel->setText(QString("Disk: %1%").arg(diskUsage, 0, 'f', 1));
    processCountLabel->setText(QString("Processes: %1").arg(processCount));
}

void MainWindow::updateProcessList()
{
    QString searchText = searchEdit->text().toLower();
    
    processTree->clear();
    
    auto processes = processManager->getProcesses();
    int displayedCount = 0;
    
    for (const auto &proc : processes) {
        if (displayedCount >= 1000) break; // Лимит для производительности
        
        if (searchText.isEmpty() || proc.name.toLower().contains(searchText)) {
            QTreeWidgetItem *item = new QTreeWidgetItem(processTree);
            item->setText(0, QString::number(proc.pid));
            item->setText(1, proc.name);
            item->setText(2, QString::number(proc.cpuUsage, 'f', 1));
            item->setText(3, QString::number(proc.memoryUsage, 'f', 1));
            item->setText(4, QString::number(proc.threadCount));
            item->setText(5, proc.userName);
            item->setText(6, proc.status);
            
            // Цветовое кодирование по загрузке CPU
            if (proc.cpuUsage > 50) {
                item->setForeground(2, QBrush(QColor(255, 100, 100)));
            } else if (proc.cpuUsage > 20) {
                item->setForeground(2, QBrush(QColor(255, 200, 100)));
            }
            
            displayedCount++;
        }
    }
    
    processTree->sortByColumn(2, Qt::DescendingOrder); // Сортировка по CPU
}

void MainWindow::updateCharts()
{
    double cpuUsage = systemMonitor->getCpuUsage();
    double ramUsage = systemMonitor->getRamUsage();
    double diskUsage = systemMonitor->getDiskUsage();
    
    // Ограничиваем историю 60 точками (1 минута)
    if (cpuData.size() > 60) cpuData.removeFirst();
    if (ramData.size() > 60) ramData.removeFirst();
    if (diskData.size() > 60) diskData.removeFirst();
    
    cpuData.append(QPointF(timeCounter, cpuUsage));
    ramData.append(QPointF(timeCounter, ramUsage));
    diskData.append(QPointF(timeCounter, diskUsage));
    
    cpuSeries->replace(cpuData);
    ramSeries->replace(ramData);
    diskSeries->replace(diskData);
    
    // Обновляем оси X
    if (timeCounter > 60) {
        cpuChart->axisX()->setRange(timeCounter - 60, timeCounter);
        ramChart->axisX()->setRange(timeCounter - 60, timeCounter);
        diskChart->axisX()->setRange(timeCounter - 60, timeCounter);
    }
    
    timeCounter++;
}

void MainWindow::onProcessSelected(QTreeWidgetItem *item, int column)
{
    if (item) {
        bool ok;
        int pid = item->text(0).toInt(&ok);
        if (ok) {
            // Можно показать детальную информацию о процессе
        }
    }
}

void MainWindow::onKillProcess()
{
    QTreeWidgetItem *item = processTree->currentItem();
    if (item) {
        bool ok;
        int pid = item->text(0).toInt(&ok);
        if (ok) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Подтверждение", 
                QString("Завершить процесс %1?").arg(pid),
                QMessageBox::Yes | QMessageBox::No);
                
            if (reply == QMessageBox::Yes) {
                if (processManager->killProcess(pid)) {
                    updateProcessList();
                } else {
                    QMessageBox::warning(this, "Ошибка", "Не удалось завершить процесс");
                }
            }
        }
    }
}

void MainWindow::onSuspendProcess()
{
    // Реализация приостановки процесса
}

void MainWindow::onResumeProcess()
{
    // Реализация возобновления процесса
}

void MainWindow::onSearchChanged(const QString &text)
{
    updateProcessList();
}

void MainWindow::onAutoUpdateToggled(bool checked)
{
    autoUpdateEnabled = checked;
}