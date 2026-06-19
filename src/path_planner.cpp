#include "path_planner.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

std::unordered_map<std::string, Point>& PathPlanner::getPointsByFloor(int floor) {
    return floor == 1 ? points_floor1 : points_floor2;
}

std::unordered_map<std::string, std::vector<Edge>>& PathPlanner::getAdjByFloor(int floor) {
    return floor == 1 ? adj_floor1 : adj_floor2;
}

bool PathPlanner::loadFloor1Data(const std::string& nodes_file, const std::string& edges_file) {
    // 加载节点数据
    std::ifstream node_file(nodes_file);
    if (!node_file.is_open()) {
        std::cerr << "无法打开节点文件: " << nodes_file << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(node_file, line); // 跳过表头
    
    while (std::getline(node_file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string name, x_str, y_str, isfloor_str;
        
        std::getline(ss, name, ',');
        std::getline(ss, x_str, ',');
        std::getline(ss, y_str, ',');
        std::getline(ss, isfloor_str, ',');
        
        try {
            int x = std::stoi(x_str);
            int y = std::stoi(y_str);
            int isfloor = std::stoi(isfloor_str);
            
            points_floor1[name] = Point(name, name, 1, x, y, isfloor);
        } catch (const std::exception& e) {
            std::cerr << "解析节点数据错误: " << line << std::endl;
        }
    }
    node_file.close();
    
    // 加载边数据
    std::ifstream edge_file(edges_file);
    if (!edge_file.is_open()) {
        std::cerr << "无法打开边文件: " << edges_file << std::endl;
        return false;
    }
    
    std::getline(edge_file, line); // 跳过表头
    
    while (std::getline(edge_file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string from, to, dist_str;
        
        std::getline(ss, from, ',');
        std::getline(ss, to, ',');
        std::getline(ss, dist_str, ',');
        
        try {
            double distance = std::stod(dist_str);
            
            // 计算方向
            std::string direction = "";
            auto it_from = points_floor1.find(from);
            auto it_to = points_floor1.find(to);
            
            if (it_from != points_floor1.end() && it_to != points_floor1.end()) {
                int dx = it_to->second.px - it_from->second.px;
                int dy = it_to->second.py - it_from->second.py;
                
                if (std::abs(dx) > std::abs(dy)) {
                    direction = dx > 0 ? "东" : "西";
                } else {
                    direction = dy > 0 ? "南" : "北";
                }
            }
            
            // 添加到邻接表（双向）
            adj_floor1[from].emplace_back(to, 1, distance, direction);
            adj_floor1[to].emplace_back(from, 1, distance, direction == "" ? "" : (direction == "东" ? "西" : direction == "西" ? "东" : direction == "南" ? "北" : "南"));
        } catch (const std::exception& e) {
            std::cerr << "解析边数据错误: " << line << std::endl;
        }
    }
    edge_file.close();
    
    return true;
}

bool PathPlanner::loadFloor2Data(const std::string& nodes_file, const std::string& edges_file) {
    // 加载节点数据
    std::ifstream node_file(nodes_file);
    if (!node_file.is_open()) {
        std::cerr << "无法打开节点文件: " << nodes_file << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(node_file, line); // 跳过表头
    
    while (std::getline(node_file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string name, x_str, y_str, isfloor_str;
        
        std::getline(ss, name, ',');
        std::getline(ss, x_str, ',');
        std::getline(ss, y_str, ',');
        std::getline(ss, isfloor_str, ',');
        
        try {
            int x = std::stoi(x_str);
            int y = std::stoi(y_str);
            int isfloor = std::stoi(isfloor_str);
            
            points_floor2[name] = Point(name, name, 2, x, y, isfloor);
        } catch (const std::exception& e) {
            std::cerr << "解析节点数据错误: " << line << std::endl;
        }
    }
    node_file.close();
    
    // 加载边数据
    std::ifstream edge_file(edges_file);
    if (!edge_file.is_open()) {
        std::cerr << "无法打开边文件: " << edges_file << std::endl;
        return false;
    }
    
    std::getline(edge_file, line); // 跳过表头
    
    while (std::getline(edge_file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string from, to, dist_str;
        
        std::getline(ss, from, ',');
        std::getline(ss, to, ',');
        std::getline(ss, dist_str, ',');
        
        try {
            double distance = std::stod(dist_str);
            
            // 计算方向
            std::string direction = "";
            auto it_from = points_floor2.find(from);
            auto it_to = points_floor2.find(to);
            
            if (it_from != points_floor2.end() && it_to != points_floor2.end()) {
                int dx = it_to->second.px - it_from->second.px;
                int dy = it_to->second.py - it_from->second.py;
                
                if (std::abs(dx) > std::abs(dy)) {
                    direction = dx > 0 ? "东" : "西";
                } else {
                    direction = dy > 0 ? "南" : "北";
                }
            }
            
            // 添加到邻接表（双向）
            adj_floor2[from].emplace_back(to, 2, distance, direction);
            adj_floor2[to].emplace_back(from, 2, distance, direction == "" ? "" : (direction == "东" ? "西" : direction == "西" ? "东" : direction == "南" ? "北" : "南"));
        } catch (const std::exception& e) {
            std::cerr << "解析边数据错误: " << line << std::endl;
        }
    }
    edge_file.close();
    
    return true;
}

bool PathPlanner::loadVirtualConnections(const std::string& virtical_file) {
    std::ifstream file(virtical_file);
    if (!file.is_open()) {
        std::cerr << "无法打开虚拟连接文件: " << virtical_file << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // 跳过表头
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string from, to, dist_str;
        
        std::getline(ss, from, ',');
        std::getline(ss, to, ',');
        std::getline(ss, dist_str, ',');
        
        // from 和 to 是同一个节点名称，表示这是一个跨楼层入口
        // 将其存储为 from -> from 的连接，表示该节点可以转换楼层
        if (!from.empty()) {
            // 添加到一楼和二楼的邻接表
            double distance = 5.0; // 默认跨楼层距离
            try {
                distance = std::stod(dist_str);
            } catch (...) {}
            
            // 一楼连接：from -> from (虚拟)
            adj_floor1[from].emplace_back(from, 2, distance, "上楼");
            
            // 二楼连接：from -> from (虚拟)
            adj_floor2[from].emplace_back(from, 1, distance, "下楼");
            
            // 存储跨楼层连接映射
            floor_connections[from] = from;
        }
    }
    file.close();
    
    return true;
}

bool PathPlanner::loadAllData() {
    bool result = true;
    result &= loadFloor1Data("data/nodes1.csv", "data/edges1.csv");
    result &= loadFloor2Data("data/nodes2.csv", "data/edges2.csv");
    result &= loadVirtualConnections("data/virtical.csv");
    return result;
}

Point PathPlanner::getPoint(const std::string& id, int floor) {
    auto& points = getPointsByFloor(floor);
    auto it = points.find(id);
    if (it != points.end()) {
        return it->second;
    }
    return Point();
}

bool PathPlanner::findPointByName(const std::string& name, std::string& out_id, int& out_floor) {
    // 先在一楼查找
    auto it1 = points_floor1.find(name);
    if (it1 != points_floor1.end()) {
        out_id = name;
        out_floor = 1;
        return true;
    }
    
    // 在二楼查找
    auto it2 = points_floor2.find(name);
    if (it2 != points_floor2.end()) {
        out_id = name;
        out_floor = 2;
        return true;
    }
    
    return false;
}

std::vector<std::string> PathPlanner::getPointIdsByFloor(int floor) {
    std::vector<std::string> ids;
    auto& points = getPointsByFloor(floor);
    for (const auto& pair : points) {
        ids.push_back(pair.first);
    }
    return ids;
}

std::vector<std::string> PathPlanner::getPointNamesByFloor(int floor) {
    return getPointIdsByFloor(floor);
}

std::vector<PathSegment> PathPlanner::findShortestPath(const std::string& start_id, int start_floor, const std::string& end_id) {
    std::vector<PathSegment> result;
    
    // 状态定义：(节点ID, 当前楼层)
    using State = std::pair<std::string, int>;
    
    struct PQItem {
        double dist;
        State state;
        std::vector<PathSegment> path;
        
        bool operator>(const PQItem& other) const {
            return dist > other.dist;
        }
    };
    
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;
    
    // 初始化：起点在指定楼层
    pq.push({0, {start_id, start_floor}, {}});
    
    std::unordered_map<std::string, double> best_cost[3]; // 分别用于一楼、二楼
    best_cost[1][start_id] = 0;
    best_cost[2][start_id] = 0;
    
    while (!pq.empty()) {
        PQItem current = pq.top();
        pq.pop();
        
        std::string current_id = current.state.first;
        int current_floor = current.state.second;
        
        // 检查是否到达终点（在任意楼层）
        if (current_id == end_id) {
            return current.path;
        }
        
        // 跳过已处理的更优路径
        if (current.dist > best_cost[current_floor][current_id]) {
            continue;
        }
        
        // 获取当前楼层的邻接表
        auto& adj = getAdjByFloor(current_floor);
        auto it_adj = adj.find(current_id);
        
        if (it_adj != adj.end()) {
            for (const Edge& edge : it_adj->second) {
                std::string next_id = edge.target_id;
                int next_floor = edge.target_floor;
                
                // 检查目标节点是否存在
                auto& target_points = getPointsByFloor(next_floor);
                if (target_points.find(next_id) == target_points.end() && next_id != current_id) {
                    continue;
                }
                
                double new_cost = current.dist + edge.distance;
                
                // 检查是否更优
                if (new_cost < best_cost[next_floor][next_id]) {
                    best_cost[next_floor][next_id] = new_cost;
                    
                    // 创建路径段
                    std::vector<PathSegment> new_path = current.path;
                    new_path.emplace_back(current_id, next_id, edge.distance, current_floor, next_floor, edge.direction);
                    
                    pq.push({new_cost, {next_id, next_floor}, new_path});
                }
            }
        }
    }
    
    return result; // 空路径表示无法到达
}

std::vector<PathSegment> PathPlanner::findShortestPathByName(const std::string& start_name, int start_floor, const std::string& end_name) {
    // 查找起点
    std::string start_id;
    int actual_start_floor;
    if (!findPointByName(start_name, start_id, actual_start_floor)) {
        return {};
    }
    
    // 如果用户指定了起始楼层，使用指定楼层
    if (start_floor != 0) {
        actual_start_floor = start_floor;
    }
    
    // 查找终点（在两个楼层都查找）
    std::string end_id = "";
    auto it1 = points_floor1.find(end_name);
    auto it2 = points_floor2.find(end_name);
    
    if (it1 != points_floor1.end()) {
        end_id = end_name;
    } else if (it2 != points_floor2.end()) {
        end_id = end_name;
    } else {
        return {};
    }
    
    // 在指定起始楼层搜索
    return findShortestPath(start_id, actual_start_floor, end_id);
}

double PathPlanner::getPathDistance(const std::vector<PathSegment>& path) {
    double total = 0;
    for (const auto& seg : path) {
        total += seg.distance;
    }
    return total;
}

std::string PathPlanner::getPathInfo(const std::vector<PathSegment>& path) {
    if (path.empty()) {
        return "无法找到路径";
    }
    
    std::stringstream ss;
    ss << "===== 路径规划结果 =====" << std::endl;
    ss << "总距离: " << getPathDistance(path) << " 米" << std::endl;
    ss << "=======================" << std::endl;
    
    std::string current_id = path[0].from_id;
    int current_floor = path[0].from_floor;
    
    ss << "起点: " << current_id << " (";
    ss << (current_floor == 1 ? "一楼" : "二楼") << ")" << std::endl;
    
    for (size_t i = 0; i < path.size(); ++i) {
        const auto& seg = path[i];
        
        // 检测楼层变化
        if (seg.from_floor != seg.to_floor) {
            ss << "  -> [跨楼层] " << (seg.to_floor == 2 ? "上楼" : "下楼") 
               << " " << seg.distance << "米" << std::endl;
        }
        
        ss << "  " << (i + 1) << ". " << seg.from_id 
           << " -> " << seg.to_id 
           << " (" << seg.distance << "米, " << seg.direction << ")" << std::endl;
        
        current_id = seg.to_id;
        current_floor = seg.to_floor;
    }
    
    ss << "终点: " << current_id << " (";
    ss << (current_floor == 1 ? "一楼" : "二楼") << ")" << std::endl;
    
    // 路径详情
    ss << std::endl << "===== 导航指引 =====" << std::endl;
    for (size_t i = 0; i < path.size(); ++i) {
        const auto& seg = path[i];
        ss << (i + 1) << ". 从 " << seg.from_id;
        if (seg.from_floor != seg.to_floor) {
            ss << " " << (seg.to_floor == 2 ? "上楼" : "下楼");
        }
        ss << " 前往 " << seg.to_id 
           << "，方向: " << seg.direction 
           << "，距离: " << seg.distance << "米" << std::endl;
    }
    
    return ss.str();
}

std::vector<std::pair<std::string, std::string>> PathPlanner::getFloorConnections() {
    std::vector<std::pair<std::string, std::string>> connections;
    for (const auto& pair : floor_connections) {
        connections.emplace_back(pair.first, pair.first);
    }
    return connections;
}

// ===== 教室导航功能实现 =====

// 根据房间号确定1楼推荐楼梯口
void getFloor1Stairs(int room_number, std::vector<std::string>& stairs) {
    stairs.clear();
    if (room_number >= 1 && room_number <= 5) {
        stairs.push_back("F9");
        stairs.push_back("F10");
    } else if (room_number >= 6 && room_number <= 9) {
        stairs.push_back("F3");
        stairs.push_back("F9");
    } else if (room_number >= 10 && room_number <= 11) {
        stairs.push_back("F3");
        stairs.push_back("F4");
    } else if (room_number >= 12 && room_number <= 18) {
        stairs.push_back("F4");
        stairs.push_back("F10");
    } else if (room_number >= 19 && room_number <= 22) {
        stairs.push_back("F7");
        stairs.push_back("F8");
    } else if (room_number >= 23 && room_number <= 31) {
        stairs.push_back("F1");
        stairs.push_back("F2");
    }
}

// 根据房间号确定2楼推荐楼梯口
void getFloor2Stairs(int room_number, std::vector<std::string>& stairs) {
    stairs.clear();
    if (room_number >= 1 && room_number <= 14) {
        stairs.push_back("F9");
        stairs.push_back("F10");
    } else if (room_number >= 15 && room_number <= 29) {
        stairs.push_back("F3");
        stairs.push_back("F4");
    } else if (room_number >= 30 && room_number <= 31) {
        stairs.push_back("F4");
        stairs.push_back("F8");
    } else if (room_number >= 32 && room_number <= 44) {
        stairs.push_back("F7");
        stairs.push_back("F8");
    } else if (room_number >= 45 && room_number <= 50) {
        stairs.push_back("F1");
        stairs.push_back("F2");
    }
}

// 根据房间号确定3楼和4楼推荐楼梯口
void getFloor3And4Stairs(int room_number, std::vector<std::string>& stairs) {
    stairs.clear();
    if (room_number >= 1 && room_number <= 14) {
        stairs.push_back("F9");
        stairs.push_back("F10");
    } else if (room_number >= 16 && room_number <= 19) {
        stairs.push_back("F9");
        stairs.push_back("F7");
    } else if (room_number >= 20 && room_number <= 25) {
        stairs.push_back("F3");
        stairs.push_back("F7");
    } else if (room_number >= 26 && room_number <= 40) {
        stairs.push_back("F3");
        stairs.push_back("F4");
    } else if (room_number >= 41 && room_number <= 42) {
        stairs.push_back("F4");
        stairs.push_back("F8");
    } else if (room_number >= 43 && room_number <= 56) {
        stairs.push_back("F7");
        stairs.push_back("F8");
    } else if (room_number >= 57 && room_number <= 69) {
        stairs.push_back("F1");
        stairs.push_back("F2");
    }
}

// 根据房间号确定5楼推荐楼梯口
void getFloor5Stairs(int room_number, std::vector<std::string>& stairs) {
    stairs.clear();
    if (room_number >= 1 && room_number <= 14) {
        stairs.push_back("F9");
        stairs.push_back("F10");
    } else if (room_number >= 16 && room_number <= 19) {
        stairs.push_back("F9");
        stairs.push_back("F7");
    } else if (room_number >= 20 && room_number <= 25) {
        stairs.push_back("F3");
        stairs.push_back("F7");
    } else if (room_number >= 26 && room_number <= 40) {
        stairs.push_back("F3");
        stairs.push_back("F4");
    } else if (room_number >= 41 && room_number <= 42) {
        stairs.push_back("F4");
        stairs.push_back("F8");
    } else if (room_number >= 43 && room_number <= 56) {
        stairs.push_back("F7");
        stairs.push_back("F8");
    } else if (room_number >= 57 && room_number <= 72) {
        stairs.push_back("F1");
        stairs.push_back("F2");
    }
}

ClassroomNavigationResult PathPlanner::parseClassroom(const std::string& classroom_input) {
    ClassroomNavigationResult result;
    result.classroom = classroom_input;
    
    // 验证输入格式：必须以 "C5-" 开头，后面跟着三位数字
    if (classroom_input.length() != 6) {
        result.valid = false;
        result.error_message = "输入格式错误！正确格式应为 C5-XXX（如 C5-101）";
        return result;
    }
    
    // 检查前缀
    std::string prefix = classroom_input.substr(0, 3);
    if (prefix != "C5-") {
        result.valid = false;
        result.error_message = "输入格式错误！必须以 C5- 开头（如 C5-101）";
        return result;
    }
    
    // 提取数字部分
    std::string num_str = classroom_input.substr(3);
    
    // 验证是否为数字
    for (char c : num_str) {
        if (!isdigit(c)) {
            result.valid = false;
            result.error_message = "教室编号必须为三位数字！";
            return result;
        }
    }
    
    int full_number = std::stoi(num_str);
    
    // 提取楼层（第一位数字）
    int floor = full_number / 100;
    
    // 提取房间号（后两位）
    int room_number = full_number % 100;
    
    // 验证楼层范围
    if (floor < 1 || floor > 5) {
        result.valid = false;
        result.error_message = "楼层编号无效！支持的楼层为 1-5 楼";
        return result;
    }
    
    result.floor = floor;
    result.room_number = room_number;
    
    // 根据楼层确定推荐楼梯口
    switch (floor) {
        case 1:
            getFloor1Stairs(room_number, result.recommended_stairs);
            break;
        case 2:
            getFloor2Stairs(room_number, result.recommended_stairs);
            break;
        case 3:
        case 4:
            getFloor3And4Stairs(room_number, result.recommended_stairs);
            break;
        case 5:
            getFloor5Stairs(room_number, result.recommended_stairs);
            break;
    }
    
    // 检查是否找到了有效的楼梯口
    if (result.recommended_stairs.empty()) {
        result.valid = false;
        result.error_message = "教室编号 " + classroom_input + " 不在有效范围内！";
        return result;
    }
    
    result.valid = true;
    result.error_message = "";
    return result;
}

std::string PathPlanner::getClassroomNavigationInfo(const std::string& classroom_input) {
    ClassroomNavigationResult result = parseClassroom(classroom_input);
    
    if (!result.valid) {
        return "[错误] " + result.error_message;
    }
    
    std::stringstream ss;
    ss << "===== C5教学楼教室导航 =====" << std::endl;
    ss << "教室编号: " << result.classroom << std::endl;
    ss << "所在楼层: " << result.floor << "楼" << std::endl;
    ss << "房间号: " << result.room_number << std::endl;
    ss << "推荐楼梯口: ";
    
    for (size_t i = 0; i < result.recommended_stairs.size(); ++i) {
        if (i > 0) {
            ss << " 和 ";
        }
        ss << result.recommended_stairs[i];
    }
    ss << std::endl;
    ss << "=============================" << std::endl;
    
    return ss.str();
}
