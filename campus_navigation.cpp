#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <climits>
#include <string>
#include <algorithm>

using namespace std;

// 边结构体：表示从起点到终点的一条边及其权重
struct Edge {
    int to;         // 目标节点索引
    int weight;     // 边的权重（距离，单位：米）
    
    Edge(int t, int w) : to(t), weight(w) {}
};

// 图类：使用邻接表存储图结构
class Graph {
private:
    vector<vector<Edge>> adjacencyList;  // 邻接表
    vector<string> nodeNames;             // 节点名称列表
    unordered_map<string, int> nameToIndex; // 节点名称到索引的映射
    
public:
    // 默认构造函数
    Graph() {}
    
    // 添加节点
    void addNode(const string& name) {
        if (nameToIndex.find(name) == nameToIndex.end()) {
            nameToIndex[name] = nodeNames.size();
            nodeNames.push_back(name);
            adjacencyList.push_back(vector<Edge>());
        }
    }
    
    // 添加无向边（双向边）
    void addUndirectedEdge(const string& from, const string& to, int weight) {
        int fromIndex = nameToIndex[from];
        int toIndex = nameToIndex[to];
        adjacencyList[fromIndex].emplace_back(toIndex, weight);
        adjacencyList[toIndex].emplace_back(fromIndex, weight);
    }
    
    // 获取节点数量
    int getNodeCount() const {
        return nodeNames.size();
    }
    
    // 根据名称获取节点索引
    int getNodeIndex(const string& name) const {
        auto it = nameToIndex.find(name);
        if (it != nameToIndex.end()) {
            return it->second;
        }
        return -1;
    }
    
    // 根据索引获取节点名称
    string getNodeName(int index) const {
        if (index >= 0 && index < nodeNames.size()) {
            return nodeNames[index];
        }
        return "";
    }
    
    // 打印图的邻接表结构
    void printGraph() const {
        cout << "图的邻接表结构：" << endl;
        for (int i = 0; i < nodeNames.size(); ++i) {
            cout << nodeNames[i] << " (" << i << "): ";
            for (const Edge& edge : adjacencyList[i]) {
                cout << nodeNames[edge.to] << "(" << edge.weight << "m) ";
            }
            cout << endl;
        }
    }
    
    // Dijkstra最短路径算法
    // 参数：start-起点索引，end-终点索引，path-用于存储路径的向量
    // 返回值：最短路径的总权重，若不存在路径返回-1
    int dijkstra(int start, int end, vector<int>& path) {
        int n = getNodeCount();
        vector<int> dist(n, INT_MAX);    // 存储到各节点的最短距离
        vector<int> prev(n, -1);         // 存储路径的前驱节点
        
        // 优先队列：存储(距离, 节点索引)，按距离升序排列
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        
        // 初始化起点
        dist[start] = 0;
        pq.push({0, start});
        
        while (!pq.empty()) {
            int currentDist = pq.top().first;
            int currentNode = pq.top().second;
            pq.pop();
            
            // 如果已到达终点，提前退出
            if (currentNode == end) {
                break;
            }
            
            // 如果当前距离大于已知最短距离，跳过
            if (currentDist > dist[currentNode]) {
                continue;
            }
            
            // 遍历当前节点的所有邻接边
            for (const Edge& edge : adjacencyList[currentNode]) {
                int nextNode = edge.to;
                int newDist = currentDist + edge.weight;
                
                // 如果找到更短的路径
                if (newDist < dist[nextNode]) {
                    dist[nextNode] = newDist;
                    prev[nextNode] = currentNode;
                    pq.push({newDist, nextNode});
                }
            }
        }
        
        // 如果终点不可达
        if (dist[end] == INT_MAX) {
            return -1;
        }
        
        // 回溯路径
        path.clear();
        for (int v = end; v != -1; v = prev[v]) {
            path.push_back(v);
        }
        reverse(path.begin(), path.end());
        
        return dist[end];
    }
    
    // 获取所有节点名称
    const vector<string>& getNodeNames() const {
        return nodeNames;
    }
};

// 从文件读取图数据
bool readGraphFromFile(const string& filename, Graph& graph) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "错误：无法打开文件 " << filename << endl;
        return false;
    }
    
    string line;
    bool isLocationSection = true;  // 当前是否在地点信息部分
    
    while (getline(file, line)) {
        // 跳过空行，作为地点信息和道路信息的分隔
        if (line.empty()) {
            isLocationSection = false;
            continue;
        }
        
        if (isLocationSection) {
            // 地点信息格式：地点名称, x坐标, y坐标
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1 + 1);
            
            if (pos1 != string::npos && pos2 != string::npos) {
                string name = line.substr(0, pos1);
                // 跳过坐标值（在最短路径算法中不需要坐标）
                graph.addNode(name);
            } else {
                cerr << "警告：无效的地点信息格式，跳过此行: " << line << endl;
            }
        } else {
            // 道路信息格式：起点名称 终点名称 距离
            size_t pos1 = line.find(' ');
            size_t pos2 = line.find(' ', pos1 + 1);
            
            if (pos1 != string::npos && pos2 != string::npos) {
                string from = line.substr(0, pos1);
                string to = line.substr(pos1 + 1, pos2 - pos1 - 1);
                int weight = stoi(line.substr(pos2 + 1));
                
                // 确保两个节点都已存在
                if (graph.getNodeIndex(from) == -1) {
                    graph.addNode(from);
                }
                if (graph.getNodeIndex(to) == -1) {
                    graph.addNode(to);
                }
                
                graph.addUndirectedEdge(from, to, weight);
            } else {
                cerr << "警告：无效的道路信息格式，跳过此行: " << line << endl;
            }
        }
    }
    
    file.close();
    return true;
}

// 显示所有可用地点
void displayLocations(const Graph& graph) {
    cout << "\n校园内可用地点：" << endl;
    const vector<string>& names = graph.getNodeNames();
    for (int i = 0; i < names.size(); ++i) {
        cout << i + 1 << ". " << names[i] << endl;
    }
}

// 主函数
int main() {
    Graph graph;
    string filename = "campus.map";
    
    // 从文件读取图数据
    cout << "正在读取校园地图文件..." << endl;
    if (!readGraphFromFile(filename, graph)) {
        cerr << "程序终止：无法读取地图文件" << endl;
        return 1;
    }
    
    cout << "地图读取成功！" << endl;
    cout << "节点数量：" << graph.getNodeCount() << endl;
    
    // 显示图结构（可选）
    // graph.printGraph();
    
    while (true) {
        // 显示所有地点
        displayLocations(graph);
        
        // 获取用户输入的起点和终点
        string startName, endName;
        cout << "\n请输入起点名称（输入 'exit' 退出程序）：";
        cin >> startName;
        
        if (startName == "exit") {
            cout << "程序结束。" << endl;
            break;
        }
        
        cout << "请输入终点名称：";
        cin >> endName;
        
        // 验证起点和终点是否存在
        int startIndex = graph.getNodeIndex(startName);
        int endIndex = graph.getNodeIndex(endName);
        
        if (startIndex == -1) {
            cout << "错误：起点 '" << startName << "' 不存在！" << endl;
            continue;
        }
        
        if (endIndex == -1) {
            cout << "错误：终点 '" << endName << "' 不存在！" << endl;
            continue;
        }
        
        if (startIndex == endIndex) {
            cout << "提示：起点和终点相同，无需计算路径。" << endl;
            continue;
        }
        
        // 使用Dijkstra算法计算最短路径
        vector<int> path;
        int shortestDist = graph.dijkstra(startIndex, endIndex, path);
        
        // 输出结果
        if (shortestDist == -1) {
            cout << "无法从 " << startName << " 到达 " << endName << endl;
        } else {
            cout << "\n从 " << startName << " 到 " << endName << " 的最短路径：" << endl;
            cout << "路径：";
            for (size_t i = 0; i < path.size(); ++i) {
                cout << graph.getNodeName(path[i]);
                if (i < path.size() - 1) {
                    cout << " -> ";
                }
            }
            cout << endl;
            cout << "总距离：" << shortestDist << " 米" << endl;
        }
        
        cout << endl;
    }
    
    return 0;
}