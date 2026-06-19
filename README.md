# 深圳技术大学校园导航系统

## 项目概述

本项目是一个跨楼层校园导航系统，采用前后端分离架构：

- **前端**：HTML/CSS/JavaScript，提供用户界面和地图可视化
- **后端**：C++，提供高性能路径规划算法和HTTP API服务

## 项目结构

```
Campus Navigation System/
├── src/                          # C++源代码
│   ├── main.cpp                  # 命令行导航程序入口
│   ├── path_planner.cpp          # 路径规划算法实现
│   ├── http_server.cpp           # HTTP服务器实现
│   └── server_main.cpp           # HTTP服务器入口
├── include/                       # C++头文件
│   ├── path_planner.h            # 路径规划类声明
│   └── http_server.h             # HTTP服务器类声明
├── data/                         # 导航数据
│   ├── nodes1.csv                # 一楼节点数据
│   ├── edges1.csv                # 一楼路径数据
│   ├── nodes2.csv                # 二楼节点数据
│   ├── edges2.csv                # 二楼路径数据
│   └── virtical.csv              # 跨楼层连接数据
├── tests/                        # 单元测试
│   └── test_path_planner.cpp     # 路径规划单元测试
├── CMakeLists.txt                # CMake构建配置
├── map.html                      # 前端导航页面
├── test_integration.bat          # 集成测试脚本
└── README.md                     # 项目说明文档
```

## 编译与运行

### 方式一：使用CMake（推荐）

```bash
# 创建build目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译所有目标
cmake --build .
```

### 方式二：直接编译

```bash
# 编译命令行导航程序
g++ -std=c++11 -o campus_navigation src/main.cpp src/path_planner.cpp

# 编译HTTP服务器
g++ -std=c++11 -o campus_server src/server_main.cpp src/http_server.cpp src/path_planner.cpp

# 编译单元测试
g++ -std=c++11 -o test_path_planner tests/test_path_planner.cpp src/path_planner.cpp
```

### 方式三：使用集成测试脚本

双击运行 `test_integration.bat`，脚本会自动编译并运行单元测试。

## 运行程序

### 命令行模式

```bash
# 运行命令行导航程序
./campus_navigation
```

### HTTP服务器模式

```bash
# 启动HTTP服务器（默认端口8080）
./campus_server

# 指定端口
./campus_server -p 8080

# 指定静态文件根目录
./campus_server -r /path/to/root
```

启动服务器后，在浏览器中访问 http://localhost:8080

## API接口

### 1. 健康检查
```
GET /api/status
```

响应：
```json
{
  "status": "ok",
  "service": "Campus Navigation API"
}
```

### 2. 获取节点列表
```
GET /api/points?floor=1
```

参数：
- `floor`: 楼层号 (1或2)

响应：
```json
{
  "floor": 1,
  "points": [
    {"id": "E-0食堂", "name": "E-0食堂"},
    {"id": "C-2图书馆", "name": "C-2图书馆"}
  ]
}
```

### 3. 获取跨楼层连接
```
GET /api/connections
```

响应：
```json
{
  "connections": [
    {"floor1": "楼梯1", "floor2": "楼梯1"},
    {"floor1": "楼梯2", "floor2": "楼梯2"}
  ]
}
```

### 4. 路径规划
```
POST /api/path
Content-Type: application/json

{
  "start": "E-0食堂",
  "start_floor": 1,
  "end": "C-00食堂"
}
```

响应：
```json
{
  "path": [
    {
      "from": "E-0食堂",
      "to": "楼梯1",
      "from_floor": 1,
      "to_floor": 1,
      "distance": 50.0,
      "direction": "北"
    },
    {
      "from": "楼梯1",
      "to": "楼梯1",
      "from_floor": 1,
      "to_floor": 2,
      "distance": 5.0,
      "direction": ""
    }
  ],
  "distance": 305.0
}
```

## 运行测试

```bash
# 运行单元测试
./test_path_planner

# 或使用测试脚本
./test_integration.bat
```

## 技术特性

### 路径规划算法
- Dijkstra最短路径算法
- 支持跨楼层导航
- 考虑楼层转换代价

### 数据结构
- 邻接表存储图结构
- 分层索引优化查询
- 双向边支持

### HTTP服务器
- 多线程处理客户端请求
- RESTful API设计
- CORS跨域支持

### 前端特性
- 响应式设计
- 跨楼层路径颜色区分（一楼翠绿色，二楼天蓝色）
- 智能搜索提示
- 地图缩放和拖拽

## 数据格式

### nodes*.csv
```
id,name,x,y,isfloor
E-0食堂,0,0,1
```

### edges*.csv
```
from,to,distance
E-0食堂,楼梯1,50.0
```

### virtical.csv
```
from,to,distance
楼梯1,楼梯1,5
```

## 常见问题

### Q: 编译时出现 "undefined reference to socket"
A: 需要链接socket库。Linux使用 `-lsocket`，Windows使用 `-lws2_32`

### Q: HTTP服务器无法启动
A: 检查端口是否被占用：`netstat -ano | findstr 8080`

### Q: 路径规划返回空
A: 检查CSV数据文件是否存在且格式正确
