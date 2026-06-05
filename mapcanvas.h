#ifndef MAPCANVAS_H
#define MAPCANVAS_H

#include <QWidget>
#include <QVector>
#include <QPoint>
#include <QString>

class Graph;

class MapCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit MapCanvas(QWidget *parent = nullptr);
    
    // 设置图数据
    void setGraph(Graph* graph);
    
    // 设置路径（用于高亮显示）
    void setPath(const QVector<int>& path);
    
    // 清除路径
    void clearPath();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Graph* m_graph;                  // 图数据指针
    QVector<int> m_path;             // 当前路径
    QPointF m_offset;                // 偏移量
    qreal m_scale;                   // 缩放比例
    int m_nodeRadius;                // 节点半径
    
    // 计算坐标变换
    QPointF transform(const QPoint& coord) const;
    
    // 更新缩放和偏移
    void updateTransform();
};

#endif // MAPCANVAS_H