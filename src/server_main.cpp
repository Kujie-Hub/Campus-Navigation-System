#include "http_server.h"
#include <iostream>
#include <csignal>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

HttpServer* g_server = nullptr;

void signalHandler(int signum) {
    if (g_server) {
        std::cout << "\nStopping server..." << std::endl;
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    int port = 8080;
    std::string root_dir = ".";
    
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
            std::cout << "Usage: server [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -p, --port <port>    Set server port (default: 8080)" << std::endl;
            std::cout << "  -r, --root <dir>     Set static file root directory (default: current)" << std::endl;
            std::cout << "  -h, --help           Show help information" << std::endl;
            return 0;
        }
    }
    
    std::cout << "======================================" << std::endl;
    std::cout << "   Campus Navigation System - Server" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Root: " << root_dir << std::endl;
    std::cout << "======================================" << std::endl;
    
    HttpServer server(port, root_dir);
    g_server = &server;
    
    if (!server.start()) {
        std::cerr << "Server start failed!" << std::endl;
        return 1;
    }
    
    std::string url = "http://localhost:" + std::to_string(port);
    std::cout << "Server started successfully!" << std::endl;
    std::cout << "Opening browser..." << std::endl;
    std::cout << "If browser does not open automatically, visit: " << url << std::endl;
    std::cout << "Press Ctrl+C to stop server." << std::endl;
    
#ifdef _WIN32
    std::string cmd = "start " + url;
    system(cmd.c_str());
#endif
    
    return 0;
}