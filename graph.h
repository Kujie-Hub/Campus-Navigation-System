#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QPoint>

// 边结构体
struct Edge {
    int to;         // 目标节点索引
    int weight;     // 边的权重（距离）
    Edge(int t, int w) : to(t), weight(w) {}
};

// 节点结构体
struct Node {
    QString name;   // 节点名称
    QPoint coord;   // 坐标位置
    Node(const QString& n, const QPoint& c) : name(n), coord(c) {}
};

// 图类：使用邻接表存储
class Graph
{
public:
    Graph();
    
    // 添加节点
    void addNode(const QString& name, int x, int y);
    
    // 添加无向边
    void addEdge(const QString& from, const QString& to, int weight);
    
    // 从文件加载图数据
    bool loadFromFile(const QString& filename);
    
    // Dijkstra算法计算最短路径
    bool dijkstra(const QString& startName, const QString& endName, QVector<int>& path);
    
    // 获取节点数量
    int nodeCount() const { return m_nodes.size(); }
    
    // 获取节点信息
    const Node& nodeAt(int index) const { return m_nodes[index]; }
    
    // 获取节点索引
    int nodeIndex(const QString& name) const;
    
    // 获取邻接表
    const QVector<QVector<Edge>>& adjacencyList() const { return m_adjacencyList; }
    
    // 获取所有节点名称
    QStringList nodeNames() const;
    
private:
    QVector<Node> m_nodes;                    // 节点列表
    QMap<QString, int> m_nameToIndex;         // 名称到索引的映射
    QVector<QVector<Edge>> m_adjacencyList;   // 邻接表
};

#endif // GRAPH_H