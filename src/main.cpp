#include "path_planner.h"
#include <iostream>
#include <iomanip>

void printColorInfo() {
    std::cout << "\n===== 路径颜色说明 =====" << std::endl;
    std::cout << "一楼路径: " << "\033[32m" << "翠绿色" << "\033[0m" 
              << " (RGB: 0, 255, 127)" << std::endl;
    std::cout << "二楼路径: " << "\033[36m" << "天蓝色" << "\033[0m" 
              << " (RGB: 135, 206, 235)" << std::endl;
    std::cout << "========================\n" << std::endl;
}

void printPathDetails(const PathPlanner& planner, const std::vector<PathSegment>& path, int start_floor) {
    if (path.empty()) {
        std::cout << "无法找到路径！" << std::endl;
        return;
    }
    
    std::cout << "\n===== 路径规划结果 =====" << std::endl;
    std::cout << "总距离: " << std::fixed << std::setprecision(2) 
              << planner.getPathDistance(path) << " 米" << std::endl;
    std::cout << "========================\n" << std::endl;
    
    std::string current_id = path[0].from_id;
    int current_floor = path[0].from_floor;
    double floor1_distance = 0;
    double floor2_distance = 0;
    
    std::cout << "起点: " << current_id << " (" 
              << (current_floor == 1 ? "一楼" : "二楼") << ")" << std::endl;
    std::cout << "------------------------" << std::endl;
    
    for (size_t i = 0; i < path.size(); ++i) {
        const auto& seg = path[i];
        
        // 统计各楼层距离
        if (seg.from_floor == 1 && seg.to_floor == 1) {
            floor1_distance += seg.distance;
        } else if (seg.from_floor == 2 && seg.to_floor == 2) {
            floor2_distance += seg.distance;
        } else if (seg.from_floor != seg.to_floor) {
            // 跨楼层，不计入任一楼层的直线距离
        }
        
        // 检测楼层变化
        if (seg.from_floor != seg.to_floor) {
            std::cout << "  |" << std::endl;
            std::cout << "  v [跨楼层] " << (seg.to_floor == 2 ? "上楼" : "下楼") 
                      << " " << seg.distance << "米" << std::endl;
            std::cout << "  |" << std::endl;
        }
        
        // 打印颜色标识
        std::string color_code = (seg.from_floor == 1) ? "\033[32m" : "\033[36m";
        std::string floor_name = (seg.from_floor == 1) ? "一楼" : "二楼";
        
        std::cout << color_code << "  " << (i + 1) << ". " << "\033[0m";
        std::cout << "从 [" << floor_name << "] " << seg.from_id;
        std::cout << " -> " << seg.to_id;
        std::cout << " (" << seg.distance << "米, " << seg.direction << ")" << std::endl;
        
        current_id = seg.to_id;
        current_floor = seg.to_floor;
    }
    
    std::cout << "------------------------" << std::endl;
    std::cout << "终点: " << current_id << " (" 
              << (current_floor == 1 ? "一楼" : "二楼") << ")" << std::endl;
    
    // 输出各楼层距离统计
    std::cout << "\n===== 距离统计 =====" << std::endl;
    if (floor1_distance > 0) {
        std::cout << "\033[32m" << "一楼距离: " << floor1_distance << " 米\033[0m" << std::endl;
    }
    if (floor2_distance > 0) {
        std::cout << "\033[36m" << "二楼距离: " << floor2_distance << " 米\033[0m" << std::endl;
    }
    
    int stair_count = 0;
    for (const auto& seg : path) {
        if (seg.from_floor != seg.to_floor) {
            stair_count++;
        }
    }
    if (stair_count > 0) {
        std::cout << "跨楼层次数: " << stair_count << std::endl;
    }
}

int main() {
    PathPlanner planner;
    
    std::cout << "===== 深圳技术大学跨楼层校园导航系统 =====" << std::endl;
    
    // 加载所有数据
    bool loaded = planner.loadAllData();
    if (!loaded) {
        std::cout << "数据加载失败！" << std::endl;
        return 1;
    }
    
    std::cout << "数据加载成功！" << std::endl;
    
    // 显示一楼节点数量
    auto floor1_points = planner.getPointNamesByFloor(1);
    auto floor2_points = planner.getPointNamesByFloor(2);
    std::cout << "一楼节点: " << floor1_points.size() << " 个" << std::endl;
    std::cout << "二楼节点: " << floor2_points.size() << " 个" << std::endl;
    
    // 显示跨楼层连接点
    auto connections = planner.getFloorConnections();
    std::cout << "跨楼层连接点: " << connections.size() << " 个" << std::endl;
    
    printColorInfo();
    
    // 测试1: 一楼内部导航
    std::cout << "\n===== 测试1: 一楼内部导航 =====" << std::endl;
    std::cout << "起点: E-4留学生外籍教师综合楼 (一楼)" << std::endl;
    std::cout << "终点: C-2图书馆" << std::endl;
    auto path1 = planner.findShortestPathByName("E-4留学生外籍教师综合楼", 1, "C-2图书馆");
    printPathDetails(planner, path1, 1);
    
    // 测试2: 二楼内部导航
    std::cout << "\n===== 测试2: 二楼内部导航 =====" << std::endl;
    std::cout << "起点: F6 (二楼)" << std::endl;
    std::cout << "终点: F1" << std::endl;
    auto path2 = planner.findShortestPathByName("F6", 2, "F1");
    printPathDetails(planner, path2, 2);
    
    // 测试3: 跨楼层导航
    std::cout << "\n===== 测试3: 跨楼层导航 =====" << std::endl;
    std::cout << "起点: E-0食堂 (一楼)" << std::endl;
    std::cout << "终点: C-00食堂" << std::endl;
    auto path3 = planner.findShortestPathByName("E-0食堂", 1, "C-00食堂");
    printPathDetails(planner, path3, 1);
    
    // 测试4: 跨楼层导航
    std::cout << "\n===== 测试4: 跨楼层导航 =====" << std::endl;
    std::cout << "起点: 马克思主义学院 (一楼)" << std::endl;
    std::cout << "终点: F5" << std::endl;
    auto path4 = planner.findShortestPathByName("马克思主义学院", 1, "F5");
    printPathDetails(planner, path4, 1);
    
    return 0;
}
