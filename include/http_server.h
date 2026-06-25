#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <atomic>

#ifdef _WIN32
    #include <windows.h>
#endif

// HTTP请求结构
struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> params;
};

// HTTP响应结构
struct HttpResponse {
    int status_code;
    std::string status_message;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    
    HttpResponse() : status_code(200), status_message("OK") {}
};

// HTTP服务器类（前向声明）
class HttpServer;

// 客户端连接上下文（用于线程传递参数）
struct ClientContext {
    HttpServer* server;
    int client_socket;
};

// HTTP服务器类
class HttpServer {
private:
    int port_;
    std::atomic<bool> running_;
    std::string root_directory_;

public:
#ifdef _WIN32
    CRITICAL_SECTION clients_mutex_;
    std::vector<int> client_sockets_;
#endif

private:
    
    // 处理客户端连接
    void handleClient(int client_socket);
    
    // 解析HTTP请求
    HttpRequest parseRequest(const std::string& raw_request);
    
    // 发送HTTP响应
    void sendResponse(int client_socket, const HttpResponse& response);
    
    // 解析URL编码的参数
    std::unordered_map<std::string, std::string> parseParams(const std::string& query);
    
    // URL解码
    std::string urlDecode(const std::string& str);
    
    // 读取文件内容
    std::string readFile(const std::string& filepath);
    
    // 获取文件MIME类型
    std::string getMimeType(const std::string& filepath);
    
    // 线程入口函数
#ifdef _WIN32
    static DWORD WINAPI clientThreadProc(LPVOID param);
#endif
    
public:
    // 构造函数
    HttpServer(int port = 8080, const std::string& root_dir = ".");
    
    // 析构函数
    ~HttpServer();
    
    // 启动服务器
    bool start();
    
    // 停止服务器
    void stop();
    
    // 检查服务器是否运行
    bool isRunning() const;
};

#endif // HTTP_SERVER_H
