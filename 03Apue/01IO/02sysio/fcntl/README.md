# 实验：Linux fcntl(2) 函数全功能实践

## 🎯 学习目标
1. 掌握 `fcntl` (File Control) 函数操作已打开的文件描述符 (FD) 的 5 大核心功能。
2. 彻底理清“文件描述符标志 (FD Flags)”与“文件状态标志 (File Status Flags)”的底层区别。
3. 理解异步 I/O (Asynchronous I/O) 的信号驱动机制与非阻塞兜底逻辑。
4. 掌握多进程安全并发的核心武器：文件记录锁 (File Record Locking)，并理解“机制与对象解耦”的设计哲学。

---

## 🧠 核心知识点与重要结论
* **读-改-写 (Read-Modify-Write) 黄金法则**：在设置 FD 标志或文件状态标志时，永远遵守 `获取 -> 按位或(|)追加 -> 设置` 的三步走策略，绝对不要直接覆盖，以防御未知的系统升级和防止误删其他并发状态。
* **锁的本质**：文件本身是没有实体锁的。加锁 (`F_SETLK/F_SETLKW`) 本质上是进程拿着捏好的锁规则 (`struct flock`)，去操作系统的“《文件独占登记册》”上进行实名登记。
* **异步与信号的羁绊**：`F_SETOWN` 只是留下了接收通知的“手机号(PID)”。在默认情况下，内核只会使用 `SIGIO` 这个特定的“短信模板”来发送 I/O 就绪通知。

---

## 📂 文件清单与功能描述

| 文件名 | 类型 | 功能描述 | 核心系统调用/宏 |
| :--- | :--- | :--- | :--- |
| `01fcntl.c` | 实验代码 | **功能 1**：复制现有的文件描述符。指定最小可用边界并分配新的 FD。 | `fcntl(fd, F_DUPFD, arg)` |
| `02fcntl.c` | 实验代码 | **功能 2**：操作文件描述符标志。演示给当前 FD 挂载/卸载“防泄漏”标志。 | `F_GETFD`, `F_SETFD`, `FD_CLOEXEC` |
| `03fcntl.c` | 实验代码 | **功能 3**：操作文件状态标志。演示如何在不重新 open 的情况下，动态追加写模式。 | `F_GETFL`, `F_SETFL`, `O_APPEND` |
| `04fcntl.c` | 实验代码 | **功能 4**：异步 I/O 驱动验证。结合非阻塞模式，演示键盘输入打断主程序的信号回调。 | `F_SETOWN`, `O_ASYNC`, `O_NONBLOCK`, `signal(SIGIO)` |
| `05fcntl.c` | 实验代码 | **功能 5**：文件记录锁竞争演示。模拟多进程抢占写锁，验证阻塞机制。 | `F_SETLKW`, `struct flock` |

---

## ⚠️ 陷阱与排错记录 (Traps & Pitfalls)
* **陷阱 1：误以为 fcntl 成功总是返回 0**
  * 排错：`cmd` 为 `F_DUPFD` 时，成功返回的是**新的文件描述符**；获取状态标志时返回的是**状态位图**；只有部分设置类操作成功才返回 `0`。
* **陷阱 2：混淆 F_SETFD 和 F_SETFL**
  * 排错：`F_SETFD` 只管当前进程的 FD 属性（目前只有 `FD_CLOEXEC`）；`F_SETFL` 管的是底层的读写属性（如 `O_NONBLOCK`, `O_APPEND`）。用混会导致严重的逻辑错误。
* **陷阱 3：只开异步不设属主**
  * 排错：如果给 FD 设置了 `O_ASYNC`，但忘记调用 `fcntl(fd, F_SETOWN, getpid())`，内核即使有了数据，也不知道该把 `SIGIO` 信号发给谁，导致程序永远收不到通知。

---

## 🚀 编译与运行指南

所有文件均可独立编译运行。建议在 Linux 终端下执行以下指令：

```bash
# 编译所有测试文件
gcc 01fcntl.c -o 01fcntl
gcc 02fcntl.c -o 02fcntl
gcc 03fcntl.c -o 03fcntl
gcc 04fcntl.c -o 04fcntl
gcc 05fcntl.c -o 05fcntl

# 运行基础功能验证
./01fcntl
./02fcntl
./03fcntl

# 运行异步 I/O 验证 (程序会持续运行，敲击键盘观察打断效果，Ctrl+C 退出)
./04fcntl

# 
./05fcntl