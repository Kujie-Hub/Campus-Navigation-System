#include "http_server.h"
#include <iostream>
#include <csignal>

HttpServer* g_server = nullptr;

void signalHandler(int signum) {
    if (g_server) {
        std::cout << "\n正在停止服务器..." << std::endl;
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    int port = 8080;
    std::string root_dir = ".";
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                port = std::stoi(argv[++i]);
            }
        } else if (arg == "-r" || arg == "--root") {
            if (i + 1 < argc) {
                root_dir = argv[++i];
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "用法: server [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  -p, --port <端口>    设置服务器端口 (默认: 8080)" << std::endl;
            std::cout << "  -r, --root <目录>   设置静态文件根目录 (默认: 当前目录)" << std::endl;
            std::cout << "  -h, --help          显示帮助信息" << std::endl;
            return 0;
        }
    }
    
    std::cout << "======================================" << std::endl;
    std::cout << "   深圳技术大学校园导航系统 - 服务器" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "端口: " << port << std::endl;
    std::cout << "根目录: " << root_dir << std::endl;
    std::cout << "======================================" << std::endl;
    
    HttpServer server(port, root_dir);
    g_server = &server;
    
    if (!server.start()) {
        std::cerr << "服务器启动失败！" << std::endl;
        return 1;
    }
    
    return 0;
}
