@echo off
chcp 65001 >nul
echo ======================================
echo    校园导航系统 - 集成测试脚本
echo ======================================
echo.

cd /d "%~dp0"

echo [1/4] 检查C++编译器...
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    where cl >nul 2>&1
    if %errorlevel% neq 0 (
        echo [错误] 未找到C++编译器 (g++ 或 cl)
        echo 请安装 MinGW-w64 或 Visual Studio
        pause
        exit /b 1
    )
    echo [OK] 找到 MSVC 编译器
    set COMPILER=msvc
) else (
    echo [OK] 找到 MinGW g++ 编译器
    set COMPILER=g++
)

echo.
echo [2/4] 编译HTTP服务器...
if "%COMPILER%"=="g++" (
    g++ -std=c++11 -o campus_server.exe src\server_main.cpp src\http_server.cpp src\path_planner.cpp -lws2_32 -lpthread 2>nul
) else (
    cl /EHsc /Fe:campus_server.exe src\server_main.cpp src\http_server.cpp src\path_planner.cpp /link ws2_32.lib 2>nul
)

if %errorlevel% neq 0 (
    echo [错误] 编译HTTP服务器失败
    pause
    exit /b 1
)
echo [OK] HTTP服务器编译成功

echo.
echo [3/4] 编译单元测试...
if "%COMPILER%"=="g++" (
    g++ -std=c++11 -o test_path_planner.exe tests\test_path_planner.cpp src\path_planner.cpp 2>nul
) else (
    cl /EHsc /Fe:test_path_planner.exe tests\test_path_planner.cpp src\path_planner.cpp 2>nul
)

if %errorlevel% neq 0 (
    echo [错误] 编译单元测试失败
    pause
    exit /b 1
)
echo [OK] 单元测试编译成功

echo.
echo [4/4] 运行单元测试...
echo.
test_path_planner.exe
set TEST_RESULT=%errorlevel%

echo.
if %TEST_RESULT% equ 0 (
    echo ======================================
    echo    所有测试通过！
    echo ======================================
) else (
    echo ======================================
    echo    部分测试失败，请检查
    echo ======================================
)

pause
exit /b %TEST_RESULT%
