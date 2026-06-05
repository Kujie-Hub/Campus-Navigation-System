// 图数据结构与Dijkstra算法

class Node {
    constructor(name, x, y) {
        this.name = name;
        this.x = x;
        this.y = y;
    }
}

class Edge {
    constructor(to, weight) {
        this.to = to;
        this.weight = weight;
    }
}

class Graph {
    constructor() {
        this.nodes = [];
        this.nameToIndex = {};
        this.adjacencyList = [];
    }

    addNode(name, x, y) {
        if (!this.nameToIndex.hasOwnProperty(name)) {
            this.nameToIndex[name] = this.nodes.length;
            this.nodes.push(new Node(name, x, y));
            this.adjacencyList.push([]);
        }
    }

    addEdge(from, to, weight) {
        const fromIndex = this.nameToIndex[from];
        const toIndex = this.nameToIndex[to];
        this.adjacencyList[fromIndex].push(new Edge(toIndex, weight));
        this.adjacencyList[toIndex].push(new Edge(fromIndex, weight));
    }

    parseMapData(data) {
        this.nodes = [];
        this.nameToIndex = {};
        this.adjacencyList = [];

        const lines = data.trim().split('\n');
        let isLocationSection = true;

        for (let line of lines) {
            line = line.trim();
            
            if (line === '') {
                isLocationSection = false;
                continue;
            }

            if (isLocationSection) {
                const parts = line.split(',');
                if (parts.length >= 3) {
                    const name = parts[0].trim();
                    const x = parseInt(parts[1].trim());
                    const y = parseInt(parts[2].trim());
                    this.addNode(name, x, y);
                }
            } else {
                const parts = line.split(/\s+/);
                if (parts.length >= 3) {
                    const from = parts[0].trim();
                    const to = parts[1].trim();
                    const weight = parseInt(parts[2].trim());
                    
                    if (!this.nameToIndex.hasOwnProperty(from)) {
                        this.addNode(from, 0, 0);
                    }
                    if (!this.nameToIndex.hasOwnProperty(to)) {
                        this.addNode(to, 0, 0);
                    }
                    
                    this.addEdge(from, to, weight);
                }
            }
        }
    }

    getNodeCount() {
        return this.nodes.length;
    }

    getNode(index) {
        return this.nodes[index];
    }

    getNodeIndex(name) {
        return this.nameToIndex.hasOwnProperty(name) ? this.nameToIndex[name] : -1;
    }

    getNodeNames() {
        return this.nodes.map(node => node.name);
    }

    dijkstra(startName, endName) {
        const start = this.getNodeIndex(startName);
        const end = this.getNodeIndex(endName);

        if (start === -1 || end === -1) {
            return null;
        }

        const n = this.getNodeCount();
        const dist = new Array(n).fill(Infinity);
        const prev = new Array(n).fill(-1);

        dist[start] = 0;
        
        const pq = new PriorityQueue((a, b) => a.dist < b.dist);
        pq.push({ node: start, dist: 0 });

        while (!pq.isEmpty()) {
            const current = pq.pop();
            const currentNode = current.node;
            const currentDist = current.dist;

            if (currentNode === end) {
                break;
            }

            if (currentDist > dist[currentNode]) {
                continue;
            }

            for (const edge of this.adjacencyList[currentNode]) {
                const newDist = currentDist + edge.weight;
                if (newDist < dist[edge.to]) {
                    dist[edge.to] = newDist;
                    prev[edge.to] = currentNode;
                    pq.push({ node: edge.to, dist: newDist });
                }
            }
        }

        if (dist[end] === Infinity) {
            return null;
        }

        const path = [];
        for (let v = end; v !== -1; v = prev[v]) {
            path.push(v);
        }
        path.reverse();

        return {
            path: path,
            distance: dist[end]
        };
    }
}

class PriorityQueue {
    constructor(compare) {
        this.heap = [];
        this.compare = compare || ((a, b) => a < b);
    }

    push(item) {
        this.heap.push(item);
        this.bubbleUp(this.heap.length - 1);
    }

    pop() {
        if (this.heap.length === 0) return null;
        if (this.heap.length === 1) return this.heap.pop();
        
        const top = this.heap[0];
        this.heap[0] = this.heap.pop();
        this.bubbleDown(0);
        return top;
    }

    isEmpty() {
        return this.heap.length === 0;
    }

    bubbleUp(index) {
        while (index > 0) {
            const parent = (index - 1) >> 1;
            if (this.compare(this.heap[parent], this.heap[index])) break;
            this.swap(parent, index);
            index = parent;
        }
    }

    bubbleDown(index) {
        const last = this.heap.length - 1;
        while (true) {
            let left = (index << 1) + 1;
            let right = left + 1;
            let minIndex = index;

            if (left <= last && this.compare(this.heap[left], this.heap[minIndex])) {
                minIndex = left;
            }
            if (right <= last && this.compare(this.heap[right], this.heap[minIndex])) {
                minIndex = right;
            }
            if (minIndex === index) break;
            this.swap(index, minIndex);
            index = minIndex;
        }
    }

    swap(i, j) {
        const temp = this.heap[i];
        this.heap[i] = this.heap[j];
        this.heap[j] = temp;
    }
}

// 校园地图数据
const campusMapData = `正门, 100, 500
图书馆, 300, 300
教学楼A, 300, 500
教学楼B, 300, 700
食堂, 500, 500
操场, 500, 700
体育馆, 700, 700
实验楼, 700, 500
学生宿舍, 700, 300
行政楼, 500, 300

正门 教学楼A 200
教学楼A 教学楼B 200
教学楼A 图书馆 200
教学楼A 食堂 200
教学楼B 操场 200
食堂 操场 200
食堂 行政楼 200
食堂 实验楼 200
操场 体育馆 200
行政楼 图书馆 200
行政楼 学生宿舍 200
实验楼 体育馆 200
实验楼 学生宿舍 200`;

// 导出
window.Graph = Graph;
window.campusMapData = campusMapData;