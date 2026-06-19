#include "http_server.h"
#include "path_planner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

HttpServer::HttpServer(int port, const std::string& root_dir)
    : port_(port), running_(false), root_directory_(root_dir) {
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    // 创建socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "创建socket失败" << std::endl;
        return false;
    }
    
    // 设置socket选项，允许地址复用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 绑定地址
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "绑定端口 " << port_ << " 失败" << std::endl;
        close(server_fd);
        return false;
    }
    
    // 开始监听
    if (listen(server_fd, 10) < 0) {
        std::cerr << "监听失败" << std::endl;
        close(server_fd);
        return false;
    }
    
    running_ = true;
    std::cout << "HTTP服务器启动成功！" << std::endl;
    std::cout << "请访问: http://localhost:" << port_ << std::endl;
    
    // 接受连接循环
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_) {
                std::cerr << "接受连接失败" << std::endl;
            }
            continue;
        }
        
        // 在新线程中处理客户端
        std::thread([this, client_socket]() {
            handleClient(client_socket);
        }).detach();
    }
    
    close(server_fd);
    return true;
}

void HttpServer::stop() {
    if (running_) {
        running_ = false;
        std::cout << "服务器已停止" << std::endl;
    }
}

bool HttpServer::isRunning() const {
    return running_;
}

void HttpServer::handleClient(int client_socket) {
    char buffer[8192] = {0};
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    std::string raw_request(buffer);
    HttpRequest request = parseRequest(raw_request);
    
    HttpResponse response;
    PathPlanner planner;
    
    // 设置CORS头
    response.headers["Access-Control-Allow-Origin"] = "*";
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS";
    response.headers["Access-Control-Allow-Headers"] = "Content-Type";
    response.headers["Content-Type"] = "application/json; charset=utf-8";
    
    // API路由
    if (request.path == "/api/points") {
        // 获取节点列表
        planner.loadAllData();
        int floor = 1;
        if (request.params.find("floor") != request.params.end()) {
            floor = std::stoi(request.params["floor"]);
        }
        
        auto names = planner.getPointNamesByFloor(floor);
        auto points = planner.getPointIdsByFloor(floor);
        
        std::ostringstream oss;
        oss << "{\"floor\":" << floor << ",\"points\":[";
        for (size_t i = 0; i < names.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "{\"id\":\"" << points[i] << "\",\"name\":\"" << names[i] << "\"}";
        }
        oss << "]}";
        response.body = oss.str();
        
    } else if (request.path == "/api/connections") {
        // 获取跨楼层连接
        planner.loadAllData();
        auto connections = planner.getFloorConnections();
        
        std::ostringstream oss;
        oss << "{\"connections\":[";
        for (size_t i = 0; i < connections.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "{\"floor1\":\"" << connections[i].first << "\",\"floor2\":\"" << connections[i].second << "\"}";
        }
        oss << "]}";
        response.body = oss.str();
        
    } else if (request.path == "/api/path") {
        // 路径规划
        planner.loadAllData();
        
        std::string start_name, end_name;
        int start_floor = 1;
        
        // 解析请求体
        if (!request.body.empty()) {
            // JSON格式: {"start":"E-0食堂","start_floor":1,"end":"C-00食堂"}
            size_t start_pos = request.body.find("\"start\"");
            if (start_pos != std::string::npos) {
                size_t colon = request.body.find(":", start_pos);
                size_t quote1 = request.body.find("\"", colon);
                size_t quote2 = request.body.find("\"", quote1 + 1);
                if (quote1 != std::string::npos && quote2 != std::string::npos) {
                    start_name = request.body.substr(quote1 + 1, quote2 - quote1 - 1);
                }
            }
            
            size_t end_pos = request.body.find("\"end\"");
            if (end_pos != std::string::npos) {
                size_t colon = request.body.find(":", end_pos);
                size_t quote1 = request.body.find("\"", colon);
                size_t quote2 = request.body.find("\"", quote1 + 1);
                if (quote1 != std::string::npos && quote2 != std::string::npos) {
                    end_name = request.body.substr(quote1 + 1, quote2 - quote1 - 1);
                }
            }
            
            size_t floor_pos = request.body.find("\"start_floor\"");
            if (floor_pos != std::string::npos) {
                size_t colon = request.body.find(":", floor_pos);
                size_t comma = request.body.find(",", colon);
                size_t brace = request.body.find("}", colon);
                size_t end = std::min(comma, brace);
                if (colon != std::string::npos && end != std::string::npos) {
                    std::string floor_str = request.body.substr(colon + 1, end - colon - 1);
                    floor_str.erase(remove_if(floor_str.begin(), floor_str.end(), ::isspace), floor_str.end());
                    if (!floor_str.empty()) {
                        start_floor = std::stoi(floor_str);
                    }
                }
            }
        }
        
        if (start_name.empty() || end_name.empty()) {
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"error\":\"缺少参数\"}";
        } else {
            auto path = planner.findShortestPathByName(start_name, start_floor, end_name);
            
            if (path.empty()) {
                response.body = "{\"path\":[],\"distance\":0,\"error\":\"无法找到路径\"}";
            } else {
                double total_distance = planner.getPathDistance(path);
                
                std::ostringstream oss;
                oss << "{\"path\":[";
                for (size_t i = 0; i < path.size(); ++i) {
                    if (i > 0) oss << ",";
                    const auto& seg = path[i];
                    oss << "{\"from\":\"" << seg.from_id << "\","
                        << "\"to\":\"" << seg.to_id << "\","
                        << "\"from_floor\":" << seg.from_floor << ","
                        << "\"to_floor\":" << seg.to_floor << ","
                        << "\"distance\":" << seg.distance << ","
                        << "\"direction\":\"" << seg.direction << "\"}";
                }
                oss << "],\"distance\":" << total_distance << "}";
                response.body = oss.str();
            }
        }
        
    } else if (request.path == "/api/classroom") {
        // 教室导航
        std::string classroom = request.params["classroom"];
        
        if (classroom.empty()) {
            // 从请求体获取
            if (!request.body.empty()) {
                size_t start_pos = request.body.find("\"classroom\"");
                if (start_pos != std::string::npos) {
                    size_t colon = request.body.find(":", start_pos);
                    size_t quote1 = request.body.find("\"", colon);
                    size_t quote2 = request.body.find("\"", quote1 + 1);
                    if (quote1 != std::string::npos && quote2 != std::string::npos) {
                        classroom = request.body.substr(quote1 + 1, quote2 - quote1 - 1);
                    }
                }
            }
        }
        
        if (classroom.empty()) {
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"error\":\"缺少教室编号参数，格式应为 C5-XXX（如 C5-101）\"}";
        } else {
            auto result = planner.parseClassroom(classroom);
            
            std::ostringstream oss;
            if (!result.valid) {
                oss << "{\"valid\":false,\"error\":\"" << result.error_message << "\"}";
            } else {
                oss << "{\"valid\":true,\"classroom\":\"" << result.classroom << "\",";
                oss << "\"floor\":" << result.floor << ",";
                oss << "\"room_number\":" << result.room_number << ",";
                oss << "\"recommended_stairs\":[";
                for (size_t i = 0; i < result.recommended_stairs.size(); ++i) {
                    if (i > 0) oss << ",";
                    oss << "\"" << result.recommended_stairs[i] << "\"";
                }
                oss << "]}";
            }
            response.body = oss.str();
        }
        
    } else if (request.path == "/api/status") {
        // 健康检查
        response.body = "{\"status\":\"ok\",\"service\":\"Campus Navigation API\"}";
        
    } else if (request.path == "/" || request.path == "/index.html") {
        // 返回HTML页面
        std::string html = readFile(root_directory_ + "/map.html");
        if (!html.empty()) {
            response.headers["Content-Type"] = "text/html; charset=utf-8";
            response.body = html;
        } else {
            response.status_code = 404;
            response.status_message = "Not Found";
            response.body = "index.html not found";
        }
        
    } else if (request.path.find("/data/") == 0) {
        // 静态数据文件
        std::string filepath = root_directory_ + request.path;
        std::string content = readFile(filepath);
        if (!content.empty()) {
            std::string mime = getMimeType(filepath);
            response.headers["Content-Type"] = mime;
            response.body = content;
        } else {
            response.status_code = 404;
            response.status_message = "Not Found";
            response.body = "File not found";
        }
        
    } else {
        // 其他静态文件
        std::string filepath = root_directory_ + request.path;
        std::string content = readFile(filepath);
        if (!content.empty()) {
            std::string mime = getMimeType(filepath);
            response.headers["Content-Type"] = mime;
            response.body = content;
        } else {
            response.status_code = 404;
            response.status_message = "Not Found";
            response.body = "404 Not Found";
        }
    }
    
    sendResponse(client_socket, response);
    close(client_socket);
}

HttpRequest HttpServer::parseRequest(const std::string& raw_request) {
    HttpRequest request;
    
    std::istringstream stream(raw_request);
    std::string line;
    
    // 解析请求行
    if (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path;
        
        // 分离路径和查询参数
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            request.query = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
        }
        
        request.params = parseParams(request.query);
    }
    
    // 解析请求头
    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty()) break;
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            // 去除空格
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \r") + 1);
            request.headers[key] = value;
        }
    }
    
    // 读取请求体
    std::ostringstream body_stream;
    body_stream << stream.rdbuf();
    request.body = body_stream.str();
    
    return request;
}

void HttpServer::sendResponse(int client_socket, const HttpResponse& response) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.status_code << " " << response.status_message << "\r\n";
    
    for (const auto& header : response.headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "Content-Length: " << response.body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << response.body;
    
    std::string response_str = oss.str();
    send(client_socket, response_str.c_str(), response_str.size(), 0);
}

std::unordered_map<std::string, std::string> HttpServer::parseParams(const std::string& query) {
    std::unordered_map<std::string, std::string> params;
    
    if (query.empty()) return params;
    
    std::istringstream stream(query);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eq_pos));
            std::string value = urlDecode(pair.substr(eq_pos + 1));
            params[key] = value;
        }
    }
    
    return params;
}

std::string HttpServer::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            std::string hex = str.substr(i + 1, 2);
            char c = static_cast<char>(std::stoi(hex, nullptr, 16));
            result += c;
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string HttpServer::readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::string HttpServer::getMimeType(const std::string& filepath) {
    std::string ext;
    size_t dot_pos = filepath.find_last_of('.');
    if (dot_pos != std::string::npos) {
        ext = filepath.substr(dot_pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }
    
    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".csv") return "text/csv; charset=utf-8";
    
    return "application/octet-stream";
}
