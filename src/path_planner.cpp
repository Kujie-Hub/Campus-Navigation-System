#include "path_planner.h"
#include <fstream>
#include <sstream>
#include <algorithm>

bool PathPlanner::loadCampusData(const std::string& campus_file) {
    std::ifstream file(campus_file);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id, name, type, floor_str, px_str, py_str;
        
        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, type, ',');
        std::getline(ss, floor_str, ',');
        std::getline(ss, px_str, ',');
        std::getline(ss, py_str, ',');
        
        int floor = std::stoi(floor_str);
        int px = std::stoi(px_str);
        int py = std::stoi(py_str);
        
        points[id] = Point(id, name, type, floor, px, py);
    }
    
    file.close();
    return true;
}

bool PathPlanner::loadPathsData(const std::string& paths_file) {
    std::ifstream file(paths_file);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string start_id, end_id, start_name, end_name, distance_str, direction, path_type;
        
        std::getline(ss, start_id, ',');
        std::getline(ss, end_id, ',');
        std::getline(ss, start_name, ',');
        std::getline(ss, end_name, ',');
        std::getline(ss, distance_str, ',');
        std::getline(ss, direction, ',');
        std::getline(ss, path_type, ',');
        
        int distance = std::stoi(distance_str);
        
        adjacency_list[start_id].emplace_back(end_id, end_name, distance, direction);
        adjacency_list[end_id].emplace_back(start_id, start_name, distance, direction);
    }
    
    file.close();
    return true;
}

std::vector<std::string> PathPlanner::findShortestPath(const std::string& start_id, const std::string& end_id) {
    if (points.find(start_id) == points.end() || points.find(end_id) == points.end()) {
        return {};
    }
    
    std::unordered_map<std::string, int> distances;
    std::unordered_map<std::string, std::string> predecessors;
    std::priority_queue<std::pair<int, std::string>, 
                        std::vector<std::pair<int, std::string>>, 
                        std::greater<std::pair<int, std::string>>> pq;
    
    for (const auto& pair : points) {
        distances[pair.first] = std::numeric_limits<int>::max();
        predecessors[pair.first] = "";
    }
    
    distances[start_id] = 0;
    pq.push({0, start_id});
    
    while (!pq.empty()) {
        auto current = pq.top();
        pq.pop();
        
        int current_dist = current.first;
        std::string current_id = current.second;
        
        if (current_id == end_id) {
            break;
        }
        
        if (current_dist > distances[current_id]) {
            continue;
        }
        
        for (const Edge& edge : adjacency_list[current_id]) {
            int new_dist = current_dist + edge.distance;
            if (new_dist < distances[edge.target_id]) {
                distances[edge.target_id] = new_dist;
                predecessors[edge.target_id] = current_id;
                pq.push({new_dist, edge.target_id});
            }
        }
    }
    
    std::vector<std::string> path;
    std::string current = end_id;
    
    while (!current.empty()) {
        path.push_back(current);
        current = predecessors[current];
        if (current == end_id) break;
    }
    
    std::reverse(path.begin(), path.end());
    
    if (path.empty() || path[0] != start_id) {
        return {};
    }
    
    return path;
}

std::vector<std::string> PathPlanner::findShortestPathByName(const std::string& start_name, const std::string& end_name) {
    std::string start_id = findIdByName(start_name);
    std::string end_id = findIdByName(end_name);
    
    if (start_id.empty() || end_id.empty()) {
        return {};
    }
    
    return findShortestPath(start_id, end_id);
}

int PathPlanner::getPathDistance(const std::vector<std::string>& path) {
    if (path.size() < 2) {
        return 0;
    }
    
    int total_distance = 0;
    
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const std::string& current = path[i];
        const std::string& next = path[i + 1];
        
        for (const Edge& edge : adjacency_list[current]) {
            if (edge.target_id == next) {
                total_distance += edge.distance;
                break;
            }
        }
    }
    
    return total_distance;
}

std::vector<std::pair<std::string, std::string>> PathPlanner::getPathDirections(const std::vector<std::string>& path) {
    std::vector<std::pair<std::string, std::string>> directions;
    
    if (path.size() < 2) {
        return directions;
    }
    
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const std::string& current = path[i];
        const std::string& next = path[i + 1];
        
        for (const Edge& edge : adjacency_list[current]) {
            if (edge.target_id == next) {
                std::string from_name = points[current].name;
                std::string to_name = edge.target_name;
                std::string dir_info = from_name + " -> " + to_name + " (" + edge.direction + ", " + std::to_string(edge.distance) + "米)";
                directions.emplace_back(edge.direction, dir_info);
                break;
            }
        }
    }
    
    return directions;
}

Point PathPlanner::getPoint(const std::string& id) const {
    auto it = points.find(id);
    if (it != points.end()) {
        return it->second;
    }
    return Point();
}

std::vector<std::string> PathPlanner::getAllPointIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : points) {
        ids.push_back(pair.first);
    }
    return ids;
}

std::vector<std::string> PathPlanner::getAllPointNames() const {
    std::vector<std::string> names;
    for (const auto& pair : points) {
        names.push_back(pair.second.name);
    }
    return names;
}

std::string PathPlanner::findIdByName(const std::string& name) const {
    for (const auto& pair : points) {
        if (pair.second.name == name) {
            return pair.first;
        }
    }
    return "";
}