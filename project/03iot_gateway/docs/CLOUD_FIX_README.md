# 云平台通信修复说明

## 问题分析

原始网关程序无法成功上传数据到云平台的原因：

1. **协议错误**：原程序直接发送JSON数据，但云平台需要HTTP协议
2. **端口错误**：原程序连接5924端口，但HTTP服务在80端口
3. **缺少认证**：原程序没有使用Base64编码的API密钥认证
4. **格式错误**：云平台需要特定的HTTP请求格式和路径

## 解决方案

参考你的STM32代码（iot.c），完全模仿其HTTP通信方式：

### 新增文件

1. **base64.h / base64.c**
   - Base64编码功能
   - 用于编码API密钥

2. **cloud_upload.h / cloud_upload.c**
   - 云平台上传模块
   - 使用HTTP协议
   - 完全参考STM32的iot.c代码

### 修改文件

1. **iot_gateway.h**
   - 添加`cloud_http_port`配置项（固定80）

2. **config.c**
   - 初始化HTTP端口为80

3. **io_multiplexing.c**
   - 包含`cloud_upload.h`
   - 修改`upload_to_cloud()`函数使用新模块
   - 删除持久连接，改为HTTP短连接

4. **Makefile**
   - 添加`base64.c`和`cloud_upload.c`到编译列表

5. **iot_gateway.conf**
   - 说明`cloud_port`实际是设备编号

## HTTP请求格式

完全参考你的STM32代码：

```http
POST /api/1.0/device/5924/datas HTTP/1.1
Host: www.embsky.com
Accept: */*
Authorization: Basic [Base64编码的API密钥]
Content-Length: [JSON数据长度]
Content-Type: application/json;charset=utf-8
Connection: close

{"datas":[{"id":16755, "data":31.8147},{"id":16753, "data":45.9632}]}
```

## 关键参数说明

1. **设备编号**：5924（你的云平台截图显示的设备编号）
2. **传感器ID**：
   - 16755: 湿度传感器
   - 16753: 温度传感器
3. **API密钥**：需要Base64编码后放在Authorization头中
4. **HTTP端口**：80（而不是5924）

## 使用方法

### 1. 重新编译

```bash
make clean
make
```

### 2. 配置文件

确保`iot_gateway.conf`中的配置正确：

```ini
# 云平台服务器
cloud_server = 119.29.98.16

# 设备编号（不是端口号！）
cloud_port = 5924

# API密钥（保留冒号）
api_key = eyJpYXQiOjE3NzMyODEzOTYsImV4cCI6MjA4ODY0MTM5NiwiYWxnIjoiSFMyNTYifQ.eyJpZCI6NTc2NX0.s42xh8K_TEXROIWCTJSHxMuA5DWPDou4gHuuPX9mMHg:
```

### 3. 运行测试

```bash
# 前台运行
./iot_gateway ./iot_gateway.conf

# 发送测试UDP数据（模拟STM32）
echo "2026-04-16 3 15:00:00 DHT:25/60 SHT:31.81/45.96 ADJ:1234" | nc -u localhost 9527
```

### 4. 查看日志

```bash
# 查看日志确认上传成功
tail -f log/gateway_$(date +%Y-%m-%d).log

# 应该看到类似输出：
# [INFO] Uploaded sensor 16755 data: 31.814700
# [INFO] Cloud upload successful
```

## 数据流程

```
STM32 (UDP) → 网关接收 → 解析数据 → 存入共享内存
                                          ↓
                            HTTP POST → 云平台 (端口80)
                            (Base64认证)
```

## 与STM32代码的对应关系

| STM32函数 | 网关函数 | 功能 |
|-----------|----------|------|
| `upload_device_datas()` | `cloud_upload_sensor_datas()` | 上传多个传感器数据 |
| `upload_sensor_data()` | `cloud_upload_single_data()` | 上传单个传感器数据 |
| `base64_encode()` | `base64_encode()` | Base64编码 |
| `esp8266_connect()` | `cloud_tcp_connect()` | TCP连接 |

## 传感器ID映射

根据云平台截图：
- 传感器16755：湿度（当前值31.8147）
- 传感器16753：温度（当前值45.9632）

网关默认上传温度数据，如需同时上传温度和湿度：

```c
// 在cloud_upload.c的cloud_upload_sensor_datas()函数中
// 构造数据时同时包含温度和湿度
```

## 预期结果

正确配置后，你应该能在云平台看到：
- 实时数据更新
- 传感器数值变化
- 时间戳更新

如果仍有问题，检查：
1. API密钥是否正确（包括冒号）
2. 设备编号是否为5924
3. 传感器ID是否匹配
4. 网络是否可达119.29.98.16:80

---

**修复日期**：2026年04月16日  
**参考代码**：STM32 iot.c  
**测试状态**：待验证
