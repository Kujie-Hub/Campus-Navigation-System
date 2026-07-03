#include "http_server.h"
#include "path_planner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    typedef int socklen_t;
    typedef int ssize_t;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define CLOSE_SOCKET close
#endif

HttpServer::HttpServer(int port, const std::string& root_dir)
    : port_(port), running_(false), root_directory_(root_dir) {
#ifdef _WIN32
    InitializeCriticalSection(&clients_mutex_);

    char module_path[MAX_PATH];
    GetModuleFileNameA(NULL, module_path, MAX_PATH);
    std::string exe_path(module_path);
    size_t last_slash = exe_path.find_last_of("\\/");
    if (last_slash != std::string::npos) {
        root_directory_ = exe_path.substr(0, last_slash);
    } else {
        root_directory_ = ".";
    }
    std::cout << "Root directory: " << root_directory_ << std::endl;
#endif
}

HttpServer::~HttpServer() {
    stop();
#ifdef _WIN32
    DeleteCriticalSection(&clients_mutex_);
#endif
}

bool HttpServer::start() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed" << std::endl;
        return false;
    }
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }
    
#ifdef _WIN32
    char opt = 1;
#else
    int opt = 1;
#endif
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind to port " << port_ << " failed" << std::endl;
        CLOSE_SOCKET(server_fd);
        return false;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        CLOSE_SOCKET(server_fd);
        return false;
    }
    
    running_ = true;
    std::cout << "HTTP server started successfully!" << std::endl;
    std::cout << "Visit: http://localhost:" << port_ << std::endl;
    
#ifdef _WIN32
    std::string url = "http://localhost:" + std::to_string(port_);
    std::string cmd = "start " + url;
    system(cmd.c_str());
    std::cout << "Browser opened automatically." << std::endl;
#endif
    
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_) {
                std::cerr << "Accept failed" << std::endl;
            }
            continue;
        }
        
#ifdef _WIN32
        ClientContext* ctx = new ClientContext();
        ctx->server = this;
        ctx->client_socket = client_socket;
        EnterCriticalSection(&clients_mutex_);
        client_sockets_.push_back(client_socket);
        LeaveCriticalSection(&clients_mutex_);
        CreateThread(NULL, 0, clientThreadProc, ctx, 0, NULL);
#else
        std::thread([this, client_socket]() {
            handleClient(client_socket);
        }).detach();
#endif
    }
    
    CLOSE_SOCKET(server_fd);
    return true;
}

void HttpServer::stop() {
    if (running_) {
        running_ = false;
        std::cout << "Server stopped" << std::endl;
    }
}

bool HttpServer::isRunning() const {
    return running_;
}

void HttpServer::handleClient(int client_socket) {
    char buffer[8192];
    std::string raw_request;
    ssize_t bytes_read = 0;
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    do {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0) {
            raw_request.append(buffer, bytes_read);
        }
    } while (bytes_read > 0 && raw_request.size() < 65536);
    
    if (raw_request.empty()) {
        CLOSE_SOCKET(client_socket);
        return;
    }
    
    HttpRequest request = parseRequest(raw_request);
    
    if (request.headers["expect"] == "100-continue") {
        std::string continue_response = "HTTP/1.1 100 Continue\r\n\r\n";
        send(client_socket, continue_response.c_str(), continue_response.size(), 0);
        
        std::string body_buffer;
        bytes_read = 0;
        do {
            memset(buffer, 0, sizeof(buffer));
            bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read > 0) {
                body_buffer.append(buffer, bytes_read);
            }
        } while (bytes_read > 0 && body_buffer.size() < 65536);
        
        if (!body_buffer.empty()) {
            request.body = body_buffer;
        }
    }
    
    HttpResponse response;
    PathPlanner planner;
    
    response.headers["Access-Control-Allow-Origin"] = "*";
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS";
    response.headers["Access-Control-Allow-Headers"] = "Content-Type";
    response.headers["Content-Type"] = "application/json; charset=utf-8";
    
    if (request.path == "/api/points") {
        planner.loadAllData();
        int floor = 1;
        if (request.params.find("floor") != request.params.end()) {
            floor = std::stoi(request.params["floor"]);
        }

        auto allPoints = planner.getAllPointsByFloor(floor);

        std::ostringstream oss;
        oss << "{\"floor\":" << floor << ",\"points\":[";
        for (size_t i = 0; i < allPoints.size(); ++i) {
            if (i > 0) oss << ",";
            const auto& p = allPoints[i];
            oss << "{\"id\":\"" << p.id << "\","
                << "\"name\":\"" << p.name << "\","
                << "\"floor\":" << p.floor << ","
                << "\"px\":" << p.px << ","
                << "\"py\":" << p.py << ","
                << "\"isfloor\":" << p.isFloor << "}";
        }
        oss << "]}";
        response.body = oss.str();
        
    } else if (request.path == "/api/connections") {
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
        planner.loadAllData();
        
        std::string start_name, end_name;
        int start_floor = 1;
        
        if (!request.body.empty()) {
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
            response.body = "{\"error\":\"Missing parameters\"}";
        } else {
            auto path = planner.findShortestPathByName(start_name, start_floor, end_name);
            
            if (path.empty()) {
                response.body = "{\"path\":[],\"distance\":0,\"error\":\"No path found\"}";
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
        std::string classroom = request.params["classroom"];
        
        if (classroom.empty()) {
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
            response.body = "{\"error\":\"Missing classroom parameter, format should be C5-XXX (e.g. C5-101)\"}";
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
        response.body = "{\"status\":\"ok\",\"service\":\"Campus Navigation API\"}";
        
    } else if (request.path == "/" || request.path == "/index.html") {
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
    CLOSE_SOCKET(client_socket);
}

HttpRequest HttpServer::parseRequest(const std::string& raw_request) {
    HttpRequest request;
    
    std::istringstream stream(raw_request);
    std::string line;
    
    if (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path;
        
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            request.query = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
        }
        
        request.params = parseParams(request.query);
    }
    
    while (std::getline(stream, line)) {
        if (line.empty() || (line.size() == 1 && line[0] == '\r')) break;
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \r") + 1);
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            request.headers[key] = value;
        }
    }

    std::string content_length = request.headers["content-length"];
    if (!content_length.empty()) {
        int len = std::stoi(content_length);
        if (len > 0) {
            char* body_buf = new char[len + 1];
            stream.read(body_buf, len);
            body_buf[len] = '\0';
            request.body = std::string(body_buf);
            delete[] body_buf;
        }
    }
    
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

#ifdef _WIN32
DWORD WINAPI HttpServer::clientThreadProc(LPVOID lpParam) {
    ClientContext* ctx = reinterpret_cast<ClientContext*>(lpParam);
    HttpServer* server = ctx->server;
    int client_socket = ctx->client_socket;
    delete ctx;
    
    server->handleClient(client_socket);
    
    EnterCriticalSection(&server->clients_mutex_);
    auto it = std::find(server->client_sockets_.begin(), server->client_sockets_.end(), client_socket);
    if (it != server->client_sockets_.end()) {
        server->client_sockets_.erase(it);
    }
    LeaveCriticalSection(&server->clients_mutex_);
    
    return 0;
}
#endif