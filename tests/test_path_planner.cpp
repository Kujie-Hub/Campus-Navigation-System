#include "../include/path_planner.h"
#include <iostream>
#include <cassert>
#include <vector>

// 测试计数器
int passed_tests = 0;
int failed_tests = 0;

void test_result(const std::string& test_name, bool success) {
    if (success) {
        std::cout << "[PASS] " << test_name << std::endl;
        passed_tests++;
    } else {
        std::cout << "[FAIL] " << test_name << std::endl;
        failed_tests++;
    }
}

// 测试1: 加载数据
void test_load_data() {
    PathPlanner planner;
    bool result = planner.loadAllData();
    test_result("加载所有数据", result);
    
    if (result) {
        auto floor1_points = planner.getPointNamesByFloor(1);
        auto floor2_points = planner.getPointNamesByFloor(2);
        test_result("一楼有节点数据", floor1_points.size() > 0);
        test_result("二楼有节点数据", floor2_points.size() > 0);
        std::cout << "  一楼节点数: " << floor1_points.size() << std::endl;
        std::cout << "  二楼节点数: " << floor2_points.size() << std::endl;
        
        auto connections = planner.getFloorConnections();
        test_result("有跨楼层连接", connections.size() > 0);
        std::cout << "  跨楼层连接数: " << connections.size() << std::endl;
    }
}

// 测试2: 获取节点
void test_get_point() {
    PathPlanner planner;
    planner.loadAllData();
    
    Point point = planner.getPoint("E-0食堂", 1);
    test_result("获取一楼E-0食堂节点", point.id == "E-0食堂");
    test_result("E-0食堂在一楼", point.floor == 1);
}

// 测试3: 查找节点
void test_find_point() {
    PathPlanner planner;
    planner.loadAllData();
    
    std::string id;
    int floor;
    bool found = planner.findPointByName("C-2图书馆", id, floor);
    test_result("通过名称找到C-2图书馆", found);
    if (found) {
        std::cout << "  找到: " << id << " (楼层 " << floor << ")" << std::endl;
    }
}

// 测试4: 一楼内部路径
void test_floor1_path() {
    PathPlanner planner;
    planner.loadAllData();
    
    auto path = planner.findShortestPathByName("E-0食堂", 1, "C-2图书馆");
    bool has_path = !path.empty();
    test_result("E-0食堂到C-2图书馆存在路径", has_path);
    
    if (has_path) {
        double dist = planner.getPathDistance(path);
        std::cout << "  路径段数: " << path.size() << std::endl;
        std::cout << "  总距离: " << dist << " 米" << std::endl;
        
        // 验证路径
        test_result("路径起点正确", path[0].from_id == "E-0食堂");
        test_result("路径终点正确", path.back().to_id == "C-2图书馆");
    }
}

// 测试5: 二楼内部路径
void test_floor2_path() {
    PathPlanner planner;
    planner.loadAllData();
    
    auto path = planner.findShortestPathByName("马克思主义学院", 2, "F5");
    bool has_path = !path.empty();
    test_result("马克思主义学院到F5存在路径", has_path);
    
    if (has_path) {
        double dist = planner.getPathDistance(path);
        std::cout << "  路径段数: " << path.size() << std::endl;
        std::cout << "  总距离: " << dist << " 米" << std::endl;
    }
}

// 测试6: 跨楼层路径（C-2图书馆到A-0食堂）
void test_cross_floor_path() {
    PathPlanner planner;
    planner.loadAllData();
    
    auto path = planner.findShortestPathByName("C-2图书馆", 1, "A-0食堂");
    bool has_path = !path.empty();
    test_result("C-2图书馆到A-0食堂存在路径", has_path);
    
    if (has_path) {
        double dist = planner.getPathDistance(path);
        std::cout << "  路径段数: " << path.size() << std::endl;
        std::cout << "  总距离: " << dist << " 米" << std::endl;
        
        // 验证路径包含跨楼层
        bool has_cross_floor = false;
        for (const auto& seg : path) {
            if (seg.from_floor != seg.to_floor) {
                has_cross_floor = true;
                break;
            }
        }
        test_result("路径包含跨楼层切换", has_cross_floor);
        
        // 打印路径详情
        std::cout << "  路径详情: ";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << path[i].from_id;
            if (path[i].from_floor != path[i].to_floor) {
                std::cout << " [跨楼层]";
            }
        }
        std::cout << " -> " << path.back().to_id << std::endl;
    }
}

// 测试7: 反向跨楼层路径
void test_cross_floor_reverse() {
    PathPlanner planner;
    planner.loadAllData();
    
    auto path = planner.findShortestPathByName("A-0食堂", 1, "C-2图书馆");
    bool has_path = !path.empty();
    test_result("A-0食堂到C-2图书馆存在路径", has_path);
    
    if (has_path) {
        double dist = planner.getPathDistance(path);
        std::cout << "  总距离: " << dist << " 米" << std::endl;
    }
}

// 测试8: 热门地点路径测试
void test_popular_routes() {
    PathPlanner planner;
    planner.loadAllData();
    
    std::vector<std::tuple<std::string, int, std::string>> test_cases = {
        {"E-0食堂", 1, "D-2体育馆(西)"},
        {"C-2图书馆", 1, "D-1城市交通与物流学院"},
        {"C-4会堂(南)", 1, "E-4留学生外籍教师综合楼"}
    };
    
    for (const auto& tc : test_cases) {
        std::string start, end;
        int floor;
        std::tie(start, floor, end) = tc;
        
        auto path = planner.findShortestPathByName(start, floor, end);
        bool has_path = !path.empty();
        std::cout << "  " << start << " -> " << end << ": ";
        if (has_path) {
            std::cout << "成功 (距离: " << planner.getPathDistance(path) << " 米)" << std::endl;
        } else {
            std::cout << "失败" << std::endl;
        }
        test_result("路径测试: " + start + " -> " + end, has_path);
    }
}

// 测试9: 路径方向正确性
void test_path_direction() {
    PathPlanner planner;
    planner.loadAllData();
    
    auto path = planner.findShortestPathByName("E-0食堂", 1, "E-1校医院");
    bool has_path = !path.empty();
    test_result("E-0食堂到E-1校医院路径", has_path);
    
    if (has_path) {
        // 检查方向字段是否有值
        bool has_direction = false;
        for (const auto& seg : path) {
            if (!seg.direction.empty()) {
                has_direction = true;
                break;
            }
        }
        test_result("路径段包含方向信息", has_direction);
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    校园导航系统 - 单元测试" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    std::cout << "--- 数据加载测试 ---" << std::endl;
    test_load_data();
    std::cout << std::endl;
    
    std::cout << "--- 节点查询测试 ---" << std::endl;
    test_get_point();
    test_find_point();
    std::cout << std::endl;
    
    std::cout << "--- 一楼路径测试 ---" << std::endl;
    test_floor1_path();
    std::cout << std::endl;
    
    std::cout << "--- 二楼路径测试 ---" << std::endl;
    test_floor2_path();
    std::cout << std::endl;
    
    std::cout << "--- 跨楼层路径测试 ---" << std::endl;
    test_cross_floor_path();
    test_cross_floor_reverse();
    std::cout << std::endl;
    
    std::cout << "--- 热门路线测试 ---" << std::endl;
    test_popular_routes();
    std::cout << std::endl;
    
    std::cout << "--- 路径属性测试 ---" << std::endl;
    test_path_direction();
    std::cout << std::endl;
    
    std::cout << "========================================" << std::endl;
    std::cout << "    测试结果汇总" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "通过: " << passed_tests << std::endl;
    std::cout << "失败: " << failed_tests << std::endl;
    std::cout << "总计: " << (passed_tests + failed_tests) << std::endl;
    
    if (failed_tests == 0) {
        std::cout << std::endl << "所有测试通过！" << std::endl;
        return 0;
    } else {
        std::cout << std::endl << "部分测试失败，请检查。" << std::endl;
        return 1;
    }
}
