# LogHawk —— 高性能分布式日志采集与处理系统

## 项目结构

```
loghawk/
├── include/
│   └── loghawk.h          # 全局头文件（所有模块共用）
├── src/
│   ├── main.c             # 入口：创建 IPC、启动三大模块、监控子进程
│   ├── collector.c        # 采集模块：监控 /var/log/syslog → 写共享内存
│   ├── processor.c        # 处理模块：读共享内存 → 解析 → 发消息队列
│   ├── outputer.c         # 输出模块：读消息队列 → 令牌桶限流 → 写文件
│   ├── ipc.c              # IPC 封装：共享内存 / 信号量 / 消息队列
│   ├── token_bucket.c     # 令牌桶限流算法
│   ├── logger.c           # 日志写出工具
│   └── utils.c            # 工具函数：偏移量、级别解析、守护进程化
├── output_logs/           # 输出目录（运行后自动创建）
│   ├── info.log           # INFO 级别日志
│   ├── warn.log           # WARN 级别日志
│   ├── error.log          # ERROR 级别日志
│   └── loghawk.log        # LogHawk 自身运行日志
└── Makefile
```

## 数据流

```
/var/log/syslog
      │
      ▼  (每秒轮询，lseek 断点续传)
 Collector
      │
      ▼  (共享内存环形缓冲区 + 信号量互斥)
 Processor × 3  (进程池)
      │
      ▼  (XSI 消息队列，按 INFO/WARN/ERROR 分类型)
  Outputer  (令牌桶限流 50条/秒)
      │
      ├──▶ output_logs/info.log
      ├──▶ output_logs/warn.log
      └──▶ output_logs/error.log
```

## 编译与运行

```bash
# 编译
make

# 方式一：读取真实系统日志（需要 sudo，因为 /var/log/syslog 权限受限）
sudo ./loghawk

# 方式二：测试模式，无需 sudo（自动生成模拟日志）
make test

# 查看实时输出
tail -f output_logs/info.log
tail -f output_logs/error.log

# 查看 LogHawk 自身运行状态
tail -f output_logs/loghawk.log

# 停止
Ctrl+C   （或）  kill $(cat /tmp/loghawk.pid)

# 清理编译产物
make clean
```

## 关于 /var/log/syslog 权限

Ubuntu 下 `/var/log/syslog` 属于 `adm` 组，普通用户没有读权限。

解决方式（选其一）：

```bash
# 方式一：sudo 运行（最简单）
sudo ./loghawk

# 方式二：把自己加入 adm 组（需重新登录生效）
sudo usermod -aG adm $USER

# 方式三：使用测试模式（无需权限）
make test
```

## 核心技术说明

| 技术点 | 用在哪里 | 为什么 |
|--------|----------|--------|
| 共享内存 | Collector → Processor | 速度最快，无拷贝 |
| 信号量 | 保护共享内存 | 防止多个工人同时读写 |
| 消息队列 | Processor → Outputer | 天然支持按类型过滤 |
| 进程池 | Processor | 多核并行，提升吞吐 |
| 令牌桶 | Outputer | 限流，保护磁盘 |
| 断点续传 | Collector | 重启后不重复读旧日志 |
| 守护进程 | Outputer（可选） | 后台长期运行 |
