// 校园导航应用主逻辑

class CampusNavigation {
    constructor() {
        this.graph = null;
        this.canvas = null;
        this.ctx = null;
        this.currentPath = [];
        this.nodeRadius = 25;
        
        this.init();
    }

    init() {
        // 初始化图数据
        this.graph = new Graph();
        this.graph.parseMapData(campusMapData);

        // 获取DOM元素
        this.canvas = document.getElementById('mapCanvas');
        this.ctx = this.canvas.getContext('2d');
        this.startSelect = document.getElementById('startSelect');
        this.endSelect = document.getElementById('endSelect');
        this.navigateBtn = document.getElementById('navigateBtn');
        this.pathPanel = document.getElementById('pathPanel');
        this.closePanelBtn = document.getElementById('closePanelBtn');
        this.pathRoute = document.getElementById('pathRoute');
        this.pathDistance = document.getElementById('pathDistance');
        this.toast = document.getElementById('toast');
        this.toastContent = this.toast.querySelector('.toast-content');

        // 填充下拉框
        this.populateSelects();

        // 绑定事件
        this.navigateBtn.addEventListener('click', () => this.onNavigate());
        this.closePanelBtn.addEventListener('click', () => this.closePathPanel());
        
        // 响应窗口大小变化
        window.addEventListener('resize', () => this.drawMap());
        
        // 初始化绘制地图
        this.resizeCanvas();
        this.drawMap();
    }

    resizeCanvas() {
        const wrapper = this.canvas.parentElement;
        const rect = wrapper.getBoundingClientRect();
        this.canvas.width = rect.width * window.devicePixelRatio;
        this.canvas.height = rect.height * window.devicePixelRatio;
        this.ctx.scale(window.devicePixelRatio, window.devicePixelRatio);
    }

    populateSelects() {
        const names = this.graph.getNodeNames();
        
        // 清空选项
        this.startSelect.innerHTML = '<option value="">请选择起点</option>';
        this.endSelect.innerHTML = '<option value="">请选择终点</option>';
        
        // 添加选项
        names.forEach(name => {
            const option1 = document.createElement('option');
            option1.value = name;
            option1.textContent = name;
            this.startSelect.appendChild(option1);

            const option2 = document.createElement('option');
            option2.value = name;
            option2.textContent = name;
            this.endSelect.appendChild(option2);
        });
    }

    onNavigate() {
        const startNode = this.startSelect.value;
        const endNode = this.endSelect.value;

        // 验证输入
        if (!startNode) {
            this.showToast('请选择起点');
            return;
        }

        if (!endNode) {
            this.showToast('请选择终点');
            return;
        }

        if (startNode === endNode) {
            this.showToast('起点和终点不能相同');
            this.currentPath = [];
            this.drawMap();
            return;
        }

        // 计算路径
        const result = this.graph.dijkstra(startNode, endNode);

        if (!result) {
            this.showToast('无法找到路径');
            this.currentPath = [];
            this.drawMap();
            return;
        }

        // 更新路径
        this.currentPath = result.path;
        
        // 显示路径面板
        this.showPathPanel(result);

        // 重绘地图
        this.drawMap();
    }

    showPathPanel(result) {
        // 获取路径名称
        const pathNames = result.path.map(index => this.graph.getNode(index).name);
        
        // 构建路径HTML
        let pathHtml = '';
        pathNames.forEach((name, index) => {
            pathHtml += `<span class="path-item">${name}</span>`;
            if (index < pathNames.length - 1) {
                pathHtml += '<span class="arrow"> -> </span>';
            }
        });
        
        this.pathRoute.innerHTML = pathHtml;
        this.pathDistance.textContent = `总距离: ${result.distance} 米`;
        
        // 显示面板
        this.pathPanel.classList.remove('hidden');
    }

    closePathPanel() {
        this.pathPanel.classList.add('hidden');
    }

    showToast(message) {
        this.toastContent.textContent = message;
        this.toast.classList.remove('hidden');
        
        setTimeout(() => {
            this.toast.classList.add('hidden');
        }, 2000);
    }

    drawMap() {
        const canvas = this.canvas;
        const ctx = this.ctx;
        const rect = canvas.parentElement.getBoundingClientRect();
        
        // 重新设置画布大小
        this.resizeCanvas();
        
        const width = rect.width;
        const height = rect.height;

        // 清空画布
        ctx.fillStyle = '#f5f5f5';
        ctx.fillRect(0, 0, width, height);

        if (!this.graph || this.graph.getNodeCount() === 0) {
            ctx.fillStyle = '#999';
            ctx.font = '14px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('暂无地图数据', width / 2, height / 2);
            return;
        }

        // 计算边界框
        let minX = Infinity, maxX = -Infinity;
        let minY = Infinity, maxY = -Infinity;

        for (let i = 0; i < this.graph.getNodeCount(); i++) {
            const node = this.graph.getNode(i);
            minX = Math.min(minX, node.x);
            maxX = Math.max(maxX, node.x);
            minY = Math.min(minY, node.y);
            maxY = Math.max(maxY, node.y);
        }

        // 添加边距
        const padding = 50;
        const contentWidth = maxX - minX + padding * 2;
        const contentHeight = maxY - minY + padding * 2;

        // 计算缩放比例
        const scaleX = (width - 40) / contentWidth;
        const scaleY = (height - 40) / contentHeight;
        const scale = Math.min(scaleX, scaleY);

        // 计算偏移
        const offsetX = width / 2 - (minX + maxX) / 2 * scale;
        const offsetY = height / 2 - (minY + maxY) / 2 * scale;

        // 绘制边
        ctx.strokeStyle = '#6495ED';
        ctx.lineWidth = 2;

        const adjList = this.graph.adjacencyList;
        for (let i = 0; i < adjList.length; i++) {
            const edges = adjList[i];
            const fromNode = this.graph.getNode(i);
            const fromX = fromNode.x * scale + offsetX;
            const fromY = fromNode.y * scale + offsetY;

            for (const edge of edges) {
                if (edge.to > i) {
                    const toNode = this.graph.getNode(edge.to);
                    const toX = toNode.x * scale + offsetX;
                    const toY = toNode.y * scale + offsetY;

                    ctx.beginPath();
                    ctx.moveTo(fromX, fromY);
                    ctx.lineTo(toX, toY);
                    ctx.stroke();

                    // 绘制距离标签
                    const midX = (fromX + toX) / 2;
                    const midY = (fromY + toY) / 2;
                    ctx.fillStyle = '#666';
                    ctx.font = '12px sans-serif';
                    ctx.textAlign = 'center';
                    ctx.fillText(edge.weight + 'm', midX, midY - 5);
                }
            }
        }

        // 绘制路径（高亮）
        if (this.currentPath.length >= 2) {
            ctx.strokeStyle = '#ff0000';
            ctx.lineWidth = 4;
            ctx.beginPath();

            const firstNode = this.graph.getNode(this.currentPath[0]);
            ctx.moveTo(firstNode.x * scale + offsetX, firstNode.y * scale + offsetY);

            for (let i = 1; i < this.currentPath.length; i++) {
                const node = this.graph.getNode(this.currentPath[i]);
                ctx.lineTo(node.x * scale + offsetX, node.y * scale + offsetY);
            }
            ctx.stroke();
        }

        // 绘制节点
        const pathSet = new Set(this.currentPath);
        
        for (let i = 0; i < this.graph.getNodeCount(); i++) {
            const node = this.graph.getNode(i);
            const x = node.x * scale + offsetX;
            const y = node.y * scale + offsetY;
            const inPath = pathSet.has(i);

            // 绘制节点圆
            ctx.beginPath();
            ctx.arc(x, y, this.nodeRadius, 0, 2 * Math.PI);
            ctx.fillStyle = inPath ? '#ff0000' : '#3CB371';
            ctx.fill();
            ctx.strokeStyle = '#000';
            ctx.lineWidth = 2;
            ctx.stroke();

            // 绘制节点名称
            ctx.fillStyle = '#000';
            ctx.font = '14px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText(node.name, x, y + this.nodeRadius + 18);
        }
    }
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', () => {
    new CampusNavigation();
});

// 导出供其他脚本使用
window.CampusNavigation = CampusNavigation;