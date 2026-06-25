@echo off
chcp 65001 >nul
REM Change to script directory
cd /d "%~dp0"

echo ======================================
echo   Campus Navigation System - Build
echo ======================================
echo.

REM Set MinGW path
set "PATH=D:\MinGW\bin;%PATH%"

REM Check compiler
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Cannot find g++. Please install MinGW to D:\MinGW
    pause
    exit /b 1
)

echo [1/3] Compiler found: OK
echo.

REM Kill old process
taskkill /F /IM http_server.exe >nul 2>&1
timeout /t 1 /nobreak >nul 2>&1

echo [2/3] Building...
g++ -std=c++11 -O2 -I include src\server_main.cpp src\http_server.cpp src\path_planner.cpp -o http_server.exe -lws2_32
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)
echo [2/3] Build success!
echo.

REM Run server
echo [3/3] Starting server...
echo.
echo ======================================
echo Server started!
echo Open browser: http://localhost:8080/
echo Press Ctrl+C to stop
echo ======================================
echo.

http_server.exe

pause
