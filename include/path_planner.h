#ifndef PATH_PLANNER_H
#define PATH_PLANNER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <limits>

struct Point {
    std::string id;
    std::string name;
    int floor;      // 楼层：1一楼，2二楼
    int px;
    int py;
    int isFloor;    // 是否是楼层入口点（用于跨楼层）
    
    Point() : floor(1), px(0), py(0), isFloor(0) {}
    Point(const std::string& i, const std::string& n, int f, int x, int y, int isF = 0)
        : id(i), name(n), floor(f), px(x), py(y), isFloor(isF) {}
};

struct Edge {
    std::string target_id;
    int target_floor;   // 目标楼层
    double distance;
    std::string direction;
    
    Edge(const std::string& tid, int tf, double dist, const std::string& dir)
        : target_id(tid), target_floor(tf), distance(dist), direction(dir) {}
};

struct PathSegment {
    std::string from_id;
    std::string to_id;
    double distance;
    int from_floor;
    int to_floor;
    std::string direction;
    
    PathSegment(const std::string& f, const std::string& t, double d, int ff, int tf, const std::string& dir)
        : from_id(f), to_id(t), distance(d), from_floor(ff), to_floor(tf), direction(dir) {}
};

// 教室导航结果结构体
struct ClassroomNavigationResult {
    bool valid;                  // 输入是否有效
    std::string error_message;   // 错误信息
    std::string classroom;       // 教室编号
    int floor;                   // 楼层
    int room_number;             // 房间号（后两位）
    std::vector<std::string> recommended_stairs;  // 推荐楼梯口
    
    ClassroomNavigationResult() : valid(false), floor(0), room_number(0) {}
};

class PathPlanner {
private:
    std::unordered_map<std::string, Point> points_floor1;      // 一楼节点
    std::unordered_map<std::string, Point> points_floor2;       // 二楼节点
    std::unordered_map<std::string, std::vector<Edge>> adj_floor1;   // 一楼邻接表
    std::unordered_map<std::string, std::vector<Edge>> adj_floor2;   // 二楼邻接表
    
    // 跨楼层连接：一楼节点ID -> 二楼节点ID
    std::unordered_map<std::string, std::string> floor_connections;
    
    // 获取某个楼层的所有节点
    std::unordered_map<std::string, Point>& getPointsByFloor(int floor);
    std::unordered_map<std::string, std::vector<Edge>>& getAdjByFloor(int floor);
    
public:
    // 加载一楼数据
    bool loadFloor1Data(const std::string& nodes_file, const std::string& edges_file);
    // 加载二楼数据
    bool loadFloor2Data(const std::string& nodes_file, const std::string& edges_file);
    // 加载跨楼层连接数据
    bool loadVirtualConnections(const std::string& virtical_file);
    
    // 综合加载函数
    bool loadAllData();
    
    // 根据楼层获取节点
    Point getPoint(const std::string& id, int floor);
    // 根据名称查找节点ID和楼层
    bool findPointByName(const std::string& name, std::string& out_id, int& out_floor);
    
    // 获取某楼层的所有节点ID
    std::vector<std::string> getPointIdsByFloor(int floor);
    // 获取某楼层的所有节点名称
    std::vector<std::string> getPointNamesByFloor(int floor);
    
    // 最优路径搜索（支持跨楼层）
    // start_id: 起点ID, start_floor: 起点楼层(1或2)
    // end_id: 终点ID（终点不需要指定楼层，会自动选择最优楼层）
    std::vector<PathSegment> findShortestPath(const std::string& start_id, int start_floor, const std::string& end_id);
    
    // 根据名称搜索最优路径
    std::vector<PathSegment> findShortestPathByName(const std::string& start_name, int start_floor, const std::string& end_name);
    
    // 获取路径总距离
    double getPathDistance(const std::vector<PathSegment>& path);
    
    // 获取路径信息
    std::string getPathInfo(const std::vector<PathSegment>& path);
    
    // 获取跨楼层节点列表
    std::vector<std::pair<std::string, std::string>> getFloorConnections();
    
    // ===== 教室导航功能 =====
    
    // 解析教室编号
    // 输入格式：C5-XXX，其中XXX为三位数字
    ClassroomNavigationResult parseClassroom(const std::string& classroom_input);
    
    // 根据教室编号获取导航信息
    std::string getClassroomNavigationInfo(const std::string& classroom_input);
};

#endif // PATH_PLANNER_H
