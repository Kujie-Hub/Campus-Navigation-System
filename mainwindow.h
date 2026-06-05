#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "graph.h"
#include "mapcanvas.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_navigateBtn_clicked();

private:
    Ui::MainWindow *ui;
    Graph m_graph;
    MapCanvas* m_mapCanvas;
    
    void initUI();
    void loadMapData();
};

#endif // MAINWINDOW_H