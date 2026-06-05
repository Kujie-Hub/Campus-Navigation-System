#include "mapcanvas.h"
#include "graph.h"
#include <QPainter>
#include <QPaintEvent>

MapCanvas::MapCanvas(QWidget *parent)
    : QWidget(parent), m_graph(nullptr), m_scale(1.0), m_nodeRadius(20)
{
    setMinimumSize(400, 300);
}

void MapCanvas::setGraph(Graph* graph)
{
    m_graph = graph;
    updateTransform();
    update();
}

void MapCanvas::setPath(const QVector<int>& path)
{
    m_path = path;
    update();
}

void MapCanvas::clearPath()
{
    m_path.clear();
    update();
}

void MapCanvas::updateTransform()
{
    if (!m_graph || m_graph->nodeCount() == 0) {
        m_offset = QPointF(50, 50);
        m_scale = 1.0;
        return;
    }

    // 计算边界框
    int minX = INT_MAX, maxX = INT_MIN;
    int minY = INT_MAX, maxY = INT_MIN;

    for (int i = 0; i < m_graph->nodeCount(); ++i) {
        const QPoint& coord = m_graph->nodeAt(i).coord;
        minX = qMin(minX, coord.x());
        maxX = qMax(maxX, coord.x());
        minY = qMin(minY, coord.y());
        maxY = qMax(maxY, coord.y());
    }

    // 添加边距
    int padding = 50;
    int contentWidth = maxX - minX + padding * 2;
    int contentHeight = maxY - minY + padding * 2;

    // 计算缩放比例
    qreal scaleX = (width() - 100) / (qreal)contentWidth;
    qreal scaleY = (height() - 100) / (qreal)contentHeight;
    m_scale = qMin(scaleX, scaleY);
    m_scale = qMax(m_scale, 0.5);  // 最小缩放

    // 计算偏移
    m_offset = QPointF(width()/2 - (minX + maxX)/2 * m_scale,
                       height()/2 - (minY + maxY)/2 * m_scale);
}

QPointF MapCanvas::transform(const QPoint& coord) const
{
    return QPointF(coord.x() * m_scale + m_offset.x(),
                   coord.y() * m_scale + m_offset.y());
}

void MapCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), QColor(245, 245, 245));

    if (!m_graph || m_graph->nodeCount() == 0) {
        painter.drawText(rect(), Qt::AlignCenter, "暂无地图数据");
        return;
    }

    // 绘制边
    painter.setPen(QPen(QColor(100, 149, 237), 2));
    const QVector<QVector<Edge>>& adjList = m_graph->adjacencyList();

    for (int i = 0; i < adjList.size(); ++i) {
        const QVector<Edge>& edges = adjList[i];
        QPointF from = transform(m_graph->nodeAt(i).coord);

        for (const Edge& edge : edges) {
            // 避免重复绘制
            if (edge.to > i) {
                QPointF to = transform(m_graph->nodeAt(edge.to).coord);
                painter.drawLine(from, to);

                // 绘制距离标签
                QPointF midPoint = (from + to) / 2;
                QString label = QString::number(edge.weight) + "m";
                painter.drawText(midPoint, label);
            }
        }
    }

    // 绘制路径（高亮）
    if (m_path.size() >= 2) {
        painter.setPen(QPen(Qt::red, 4, Qt::SolidLine));
        for (int i = 0; i < m_path.size() - 1; ++i) {
            QPointF from = transform(m_graph->nodeAt(m_path[i]).coord);
            QPointF to = transform(m_graph->nodeAt(m_path[i+1]).coord);
            painter.drawLine(from, to);
        }
    }

    // 绘制节点
    for (int i = 0; i < m_graph->nodeCount(); ++i) {
        const Node& node = m_graph->nodeAt(i);
        QPointF pos = transform(node.coord);

        // 判断是否在路径中
        bool inPath = m_path.contains(i);
        QColor color = inPath ? Qt::red : QColor(60, 179, 113);

        // 绘制节点圆
        painter.setBrush(QBrush(color));
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(pos, m_nodeRadius, m_nodeRadius);

        // 绘制节点名称
        painter.setPen(QPen(Qt::black));
        painter.setFont(QFont("Arial", 10));
        painter.drawText(QRectF(pos.x() - m_nodeRadius, pos.y() + m_nodeRadius + 5,
                                m_nodeRadius * 2, 20),
                         Qt::AlignCenter, node.name);
    }
}