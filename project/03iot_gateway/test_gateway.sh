#!/bin/bash
# ================================================
# IoT Gateway 测试脚本
# 用于快速测试网关各项功能
# ================================================

echo "=================================="
echo "  IoT Gateway 功能测试脚本"
echo "=================================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查网关是否运行
check_gateway() {
    if [ -f /var/run/iot_gateway.pid ]; then
        echo -e "${GREEN}✓ 网关正在运行${NC}"
        return 0
    else
        echo -e "${RED}✗ 网关未运行${NC}"
        return 1
    fi
}

# 测试1: UDP发送测试数据
test_udp() {
    echo ""
    echo "【测试1】UDP数据发送测试"
    echo "发送模拟传感器数据到UDP端口9527..."
    
    # 模拟STM32发送的传感器数据
    TEST_DATA="2026-04-15 2 14:30:45 DHT:25/60 SHT:25.50/60.20 ADJ:1234"
    
    echo "$TEST_DATA" | nc -u -w1 localhost 9527
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ UDP数据发送成功${NC}"
        echo "  数据内容: $TEST_DATA"
    else
        echo -e "${RED}✗ UDP数据发送失败${NC}"
    fi
    
    sleep 1
}

# 测试2: TCP连接测试
test_tcp() {
    echo ""
    echo "【测试2】TCP连接测试"
    echo "连接到TCP端口8080..."
    
    (
        echo "iot_gateway_2024"
        sleep 1
        echo "hello gateway"
        sleep 1
    ) | telnet localhost 8080 > /tmp/tcp_test.log 2>&1 &
    
    sleep 3
    
    if grep -q "AUTH_OK" /tmp/tcp_test.log; then
        echo -e "${GREEN}✓ TCP认证成功${NC}"
    else
        echo -e "${YELLOW}⚠ TCP认证可能失败（需要检查日志）${NC}"
    fi
    
    rm -f /tmp/tcp_test.log
}

# 测试3: 连续发送多个UDP包（测试限流）
test_rate_limit() {
    echo ""
    echo "【测试3】令牌桶限流测试"
    echo "快速发送200个UDP数据包..."
    
    for i in {1..200}; do
        echo "2026-04-15 2 14:30:$i DHT:$i/60 SHT:25.50/60.20 ADJ:$i" | nc -u -w0 localhost 9527 &
    done
    
    wait
    echo -e "${GREEN}✓ 已发送200个数据包${NC}"
    echo "  检查日志应该能看到限流丢包信息"
    sleep 2
}

# 测试4: 查看共享内存
test_shared_memory() {
    echo ""
    echo "【测试4】共享内存测试"
    echo "查看IPC资源..."
    
    echo "消息队列:"
    ipcs -q | grep $(whoami) || echo "  无消息队列"
    
    echo ""
    echo "共享内存:"
    ipcs -m | grep $(whoami) || echo "  无共享内存"
    
    echo ""
    echo "信号量:"
    ipcs -s | grep $(whoami) || echo "  无信号量"
}

# 测试5: 查看日志
test_logs() {
    echo ""
    echo "【测试5】日志系统测试"
    
    LOG_FILE="log/gateway_$(date +%Y-%m-%d).log"
    
    if [ -f "$LOG_FILE" ]; then
        echo -e "${GREEN}✓ 日志文件存在: $LOG_FILE${NC}"
        echo ""
        echo "最近10条日志:"
        echo "----------------------------------------"
        tail -n 10 "$LOG_FILE"
        echo "----------------------------------------"
    else
        echo -e "${RED}✗ 日志文件不存在${NC}"
    fi
}

# 测试6: 性能测试
test_performance() {
    echo ""
    echo "【测试6】性能测试"
    echo "检查进程资源占用..."
    
    if [ -f /var/run/iot_gateway.pid ]; then
        PID=$(cat /var/run/iot_gateway.pid)
        echo "进程状态:"
        ps -p $PID -o pid,ppid,%cpu,%mem,vsz,rss,etime,cmd
        
        echo ""
        echo "TCP连接数:"
        netstat -an | grep :8080 | grep ESTABLISHED | wc -l
        
        echo ""
        echo "线程数:"
        ps -T -p $PID | wc -l
    else
        echo -e "${YELLOW}⚠ 网关未运行，跳过性能测试${NC}"
    fi
}

# 主菜单
show_menu() {
    echo ""
    echo "请选择测试项目:"
    echo "  1) UDP数据发送测试"
    echo "  2) TCP连接测试"
    echo "  3) 令牌桶限流测试"
    echo "  4) 共享内存测试"
    echo "  5) 日志系统测试"
    echo "  6) 性能测试"
    echo "  7) 运行所有测试"
    echo "  0) 退出"
    echo ""
    read -p "请输入选项 [0-7]: " choice
    
    case $choice in
        1) test_udp ;;
        2) test_tcp ;;
        3) test_rate_limit ;;
        4) test_shared_memory ;;
        5) test_logs ;;
        6) test_performance ;;
        7) 
            test_udp
            test_tcp
            test_rate_limit
            test_shared_memory
            test_logs
            test_performance
            ;;
        0) 
            echo "退出测试"
            exit 0
            ;;
        *)
            echo -e "${RED}无效选项${NC}"
            ;;
    esac
    
    show_menu
}

# 检查依赖
check_dependencies() {
    echo "检查依赖工具..."
    
    # 检查nc
    if ! command -v nc &> /dev/null; then
        echo -e "${RED}✗ 未安装 nc (netcat)${NC}"
        echo "  请安装: sudo yum install nc 或 sudo apt install netcat"
        exit 1
    fi
    
    # 检查telnet
    if ! command -v telnet &> /dev/null; then
        echo -e "${YELLOW}⚠ 未安装 telnet（TCP测试将跳过）${NC}"
    fi
    
    echo -e "${GREEN}✓ 依赖检查完成${NC}"
}

# 主程序
main() {
    check_dependencies
    
    echo ""
    check_gateway
    
    if [ "$1" == "auto" ]; then
        # 自动运行所有测试
        test_udp
        test_tcp
        test_rate_limit
        test_shared_memory
        test_logs
        test_performance
        
        echo ""
        echo "=================================="
        echo "  所有测试完成"
        echo "=================================="
    else
        # 交互式菜单
        show_menu
    fi
}

# 运行主程序
main "$@"
