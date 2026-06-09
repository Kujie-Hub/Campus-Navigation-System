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
    std::string type;
    int floor;
    int px;
    int py;
    
    Point() : floor(0), px(0), py(0) {}
    Point(const std::string& i, const std::string& n, const std::string& t, int f, int x, int y)
        : id(i), name(n), type(t), floor(f), px(x), py(y) {}
};

struct Edge {
    std::string target_id;
    std::string target_name;
    int distance;
    std::string direction;
    
    Edge(const std::string& tid, const std::string& tname, int dist, const std::string& dir)
        : target_id(tid), target_name(tname), distance(dist), direction(dir) {}
};

class PathPlanner {
private:
    std::unordered_map<std::string, Point> points;
    std::unordered_map<std::string, std::vector<Edge>> adjacency_list;
    
public:
    bool loadCampusData(const std::string& campus_file);
    bool loadPathsData(const std::string& paths_file);
    
    std::vector<std::string> findShortestPath(const std::string& start_id, const std::string& end_id);
    std::vector<std::string> findShortestPathByName(const std::string& start_name, const std::string& end_name);
    
    int getPathDistance(const std::vector<std::string>& path);
    std::vector<std::pair<std::string, std::string>> getPathDirections(const std::vector<std::string>& path);
    
    Point getPoint(const std::string& id) const;
    std::vector<std::string> getAllPointIds() const;
    std::vector<std::string> getAllPointNames() const;
    std::string findIdByName(const std::string& name) const;
};

#endif // PATH_PLANNER_H