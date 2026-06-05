#include "graph.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <climits>
#include <queue>
#include <algorithm>

using namespace std;

Graph::Graph()
{
}

void Graph::addNode(const QString& name, int x, int y)
{
    if (!m_nameToIndex.contains(name)) {
        m_nameToIndex[name] = m_nodes.size();
        m_nodes.append(Node(name, QPoint(x, y)));
        m_adjacencyList.append(QVector<Edge>());
    }
}

void Graph::addEdge(const QString& from, const QString& to, int weight)
{
    int fromIndex = m_nameToIndex[from];
    int toIndex = m_nameToIndex[to];
    m_adjacencyList[fromIndex].append(Edge(toIndex, weight));
    m_adjacencyList[toIndex].append(Edge(fromIndex, weight));
}

bool Graph::loadFromFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", "无法打开文件: " + filename);
        return false;
    }

    m_nodes.clear();
    m_nameToIndex.clear();
    m_adjacencyList.clear();

    QTextStream in(&file);
    bool isLocationSection = true;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // 空行作为分隔
        if (line.isEmpty()) {
            isLocationSection = false;
            continue;
        }

        if (isLocationSection) {
            // 地点信息格式：名称, x, y
            QStringList parts = line.split(',');
            if (parts.size() >= 3) {
                QString name = parts[0].trimmed();
                int x = parts[1].trimmed().toInt();
                int y = parts[2].trimmed().toInt();
                addNode(name, x, y);
            }
        } else {
            // 道路信息格式：起点 终点 距离
            QStringList parts = line.split(' ', QString::SkipEmptyParts);
            if (parts.size() >= 3) {
                QString from = parts[0].trimmed();
                QString to = parts[1].trimmed();
                int weight = parts[2].trimmed().toInt();
                
                // 确保节点存在
                if (!m_nameToIndex.contains(from)) {
                    addNode(from, 0, 0);
                }
                if (!m_nameToIndex.contains(to)) {
                    addNode(to, 0, 0);
                }
                
                addEdge(from, to, weight);
            }
        }
    }

    file.close();
    return true;
}

bool Graph::dijkstra(const QString& startName, const QString& endName, QVector<int>& path)
{
    int start = nodeIndex(startName);
    int end = nodeIndex(endName);

    if (start == -1 || end == -1) {
        return false;
    }

    int n = nodeCount();
    QVector<int> dist(n, INT_MAX);
    QVector<int> prev(n, -1);

    // 优先队列：(距离, 节点)
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    dist[start] = 0;
    pq.push(make_pair(0, start));

    while (!pq.empty()) {
        int currentDist = pq.top().first;
        int currentNode = pq.top().second;
        pq.pop();

        if (currentNode == end) {
            break;
        }

        if (currentDist > dist[currentNode]) {
            continue;
        }

        for (const Edge& edge : m_adjacencyList[currentNode]) {
            int newDist = currentDist + edge.weight;
            if (newDist < dist[edge.to]) {
                dist[edge.to] = newDist;
                prev[edge.to] = currentNode;
                pq.push(make_pair(newDist, edge.to));
            }
        }
    }

    if (dist[end] == INT_MAX) {
        return false;
    }

    // 回溯路径
    path.clear();
    for (int v = end; v != -1; v = prev[v]) {
        path.append(v);
    }
    reverse(path.begin(), path.end());

    return true;
}

int Graph::nodeIndex(const QString& name) const
{
    if (m_nameToIndex.contains(name)) {
        return m_nameToIndex[name];
    }
    return -1;
}

QStringList Graph::nodeNames() const
{
    QStringList names;
    for (const Node& node : m_nodes) {
        names.append(node.name);
    }
    return names;
}