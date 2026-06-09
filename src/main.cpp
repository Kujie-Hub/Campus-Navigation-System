#include "path_planner.h"
#include <iostream>

int main() {
    PathPlanner planner;
    
    bool campus_loaded = planner.loadCampusData("data/campus.csv");
    bool paths_loaded = planner.loadPathsData("data/paths.csv");
    
    if (!campus_loaded || !paths_loaded) {
        std::cout << "Error loading data files!" << std::endl;
        return 1;
    }
    
    std::cout << "===== 深圳技术大学校园导航系统 =====" << std::endl;
    std::cout << "数据加载成功！" << std::endl;
    
    std::vector<std::string> path = planner.findShortestPathByName("会堂东门", "南区宿舍");
    
    if (path.empty()) {
        std::cout << "无法找到路径！" << std::endl;
        return 1;
    }
    
    std::cout << "\n最短路径规划结果：" << std::endl;
    std::cout << "---------------------" << std::endl;
    
    for (size_t i = 0; i < path.size(); ++i) {
        Point p = planner.getPoint(path[i]);
        std::cout << (i + 1) << ". " << p.name << " (" << p.id << ")" << std::endl;
    }
    
    int distance = planner.getPathDistance(path);
    std::cout << "\n总距离：" << distance << " 米" << std::endl;
    
    std::vector<std::pair<std::string, std::string>> directions = planner.getPathDirections(path);
    std::cout << "\n导航指引：" << std::endl;
    std::cout << "---------------------" << std::endl;
    
    for (size_t i = 0; i < directions.size(); ++i) {
        std::cout << (i + 1) << ". " << directions[i].second << std::endl;
    }
    
    return 0;
}