#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUI();
    loadMapData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    // 设置窗口大小
    resize(800, 600);

    // 创建中心部件
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 创建地图画布
    m_mapCanvas = new MapCanvas(this);
    m_mapCanvas->setMinimumSize(400, 300);
    mainLayout->addWidget(m_mapCanvas, 1);

    // 创建控制面板
    QWidget* controlPanel = new QWidget(this);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(10);

    // 创建起点下拉框
    QLabel* startLabel = new QLabel("起点:", this);
    QComboBox* startCombo = new QComboBox(this);
    startCombo->setObjectName("startCombo");
    startCombo->setMinimumWidth(120);

    // 创建终点下拉框
    QLabel* endLabel = new QLabel("终点:", this);
    QComboBox* endCombo = new QComboBox(this);
    endCombo->setObjectName("endCombo");
    endCombo->setMinimumWidth(120);

    // 创建导航按钮
    QPushButton* navigateBtn = new QPushButton("导航", this);
    navigateBtn->setObjectName("navigateBtn");
    navigateBtn->setMinimumWidth(80);
    connect(navigateBtn, &QPushButton::clicked, this, &MainWindow::on_navigateBtn_clicked);

    // 添加到布局
    controlLayout->addWidget(startLabel);
    controlLayout->addWidget(startCombo);
    controlLayout->addWidget(endLabel);
    controlLayout->addWidget(endCombo);
    controlLayout->addWidget(navigateBtn);
    controlLayout->addStretch();

    mainLayout->addWidget(controlPanel);
}

void MainWindow::loadMapData()
{
    QString filename = "campus.map";
    if (m_graph.loadFromFile(filename)) {
        m_mapCanvas->setGraph(&m_graph);

        // 更新下拉框选项
        QComboBox* startCombo = findChild<QComboBox*>("startCombo");
        QComboBox* endCombo = findChild<QComboBox*>("endCombo");
        
        if (startCombo && endCombo) {
            QStringList names = m_graph.nodeNames();
            startCombo->addItems(names);
            endCombo->addItems(names);
        }
    } else {
        QMessageBox::critical(this, "错误", "无法加载地图数据文件");
    }
}

void MainWindow::on_navigateBtn_clicked()
{
    QComboBox* startCombo = findChild<QComboBox*>("startCombo");
    QComboBox* endCombo = findChild<QComboBox*>("endCombo");

    if (!startCombo || !endCombo) {
        QMessageBox::warning(this, "错误", "控件初始化失败");
        return;
    }

    QString startName = startCombo->currentText();
    QString endName = endCombo->currentText();

    // 验证输入
    if (startName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择起点");
        return;
    }

    if (endName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择终点");
        return;
    }

    if (startName == endName) {
        QMessageBox::warning(this, "提示", "起点和终点不能相同");
        m_mapCanvas->clearPath();
        return;
    }

    // 计算路径
    QVector<int> path;
    if (m_graph.dijkstra(startName, endName, path)) {
        // 显示路径
        m_mapCanvas->setPath(path);

        // 计算总距离
        int totalDistance = 0;
        for (int i = 0; i < path.size() - 1; ++i) {
            int from = path[i];
            int to = path[i+1];
            
            const QVector<Edge>& edges = m_graph.adjacencyList()[from];
            for (const Edge& edge : edges) {
                if (edge.to == to) {
                    totalDistance += edge.weight;
                    break;
                }
            }
        }

        // 显示路径信息
        QString pathStr;
        for (int i = 0; i < path.size(); ++i) {
            pathStr += m_graph.nodeAt(path[i]).name;
            if (i < path.size() - 1) {
                pathStr += " -> ";
            }
        }

        QMessageBox::information(this, "导航结果",
                                QString("最短路径:\n%1\n总距离: %2 米").arg(pathStr).arg(totalDistance));
    } else {
        QMessageBox::warning(this, "提示", QString("无法找到从 %1 到 %2 的路径").arg(startName).arg(endName));
        m_mapCanvas->clearPath();
    }
}