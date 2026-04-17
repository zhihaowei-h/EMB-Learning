/*===============================================
 *   文件名称：config.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：配置文件解析模块
 *            支持键值对配置、注释、空行过滤
 ================================================*/

#include "../inc/iot_gateway.h"

// 全局配置变量定义
config_t g_config;

/*
 * 功能：去除字符串首尾空白
 * 参数：str - 待处理字符串
 * 返回：处理后的字符串指针
 */
static char *trim(char *str)
{
    char *end;
    
    // 去除前导空白
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    // 如果全是空白，返回空字符串
    if (*str == 0) {
        return str;
    }
    
    // 去除尾部空白
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    *(end + 1) = '\0';
    
    return str;
}

/*
 * 功能：解析配置行
 * 参数：line - 配置行
 *       key - 输出键
 *       value - 输出值
 * 返回：成功返回0，失败返回-1
 */
static int parse_line(char *line, char *key, char *value)
{
    char *eq_pos;
    
    // 去除首尾空白
    line = trim(line);
    
    // 跳过空行和注释行
    if (line[0] == '\0' || line[0] == '#') {
        return -1;
    }
    
    // 查找等号
    eq_pos = strchr(line, '=');
    if (eq_pos == NULL) {
        return -1;
    }
    
    // 分割键和值
    *eq_pos = '\0';
    strcpy(key, trim(line));
    strcpy(value, trim(eq_pos + 1));
    
    return 0;
}

/*
 * 功能：加载配置文件
 * 参数：config_file - 配置文件路径
 * 返回：成功返回0，失败返回-1
 */
int config_load(const char *config_file)
{
    FILE *fp;
    char line[CONFIG_LINE_SIZE];
    char key[128], value[256];
    
    // 设置默认值
    g_config.tcp_port = 8080;
    g_config.udp_port = 8081;
    g_config.log_level = LOG_INFO;
    strcpy(g_config.log_path, "./log");
    g_config.thread_pool_size = 10;
    g_config.process_pool_size = 3;
    g_config.token_cps = 100;
    g_config.token_burst = 200;
    strcpy(g_config.auth_token, "iot_gateway_2024");
    strcpy(g_config.api_key, "");
    strcpy(g_config.cloud_server, CLOUD_SERVER);
    g_config.cloud_port = CLOUD_PORT;
    g_config.cloud_http_port = 80;  // HTTP端口固定为80
    
    // 打开配置文件
    fp = fopen(config_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "[WARN] Config file not found: %s, using defaults\n", config_file);
        return 0;  // 使用默认值继续运行
    }
    
    // 逐行读取配置
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (parse_line(line, key, value) < 0) {
            continue;
        }
        
        // 解析各个配置项
        if (strcmp(key, "tcp_port") == 0) {
            g_config.tcp_port = atoi(value);
        } else if (strcmp(key, "udp_port") == 0) {
            g_config.udp_port = atoi(value);
        } else if (strcmp(key, "log_level") == 0) {
            if (strcmp(value, "DEBUG") == 0) {
                g_config.log_level = LOG_DEBUG;
            } else if (strcmp(value, "INFO") == 0) {
                g_config.log_level = LOG_INFO;
            } else if (strcmp(value, "WARN") == 0) {
                g_config.log_level = LOG_WARN;
            } else if (strcmp(value, "ERROR") == 0) {
                g_config.log_level = LOG_ERROR;
            }
        } else if (strcmp(key, "log_path") == 0) {
            strncpy(g_config.log_path, value, sizeof(g_config.log_path) - 1);
        } else if (strcmp(key, "thread_pool_size") == 0) {
            g_config.thread_pool_size = atoi(value);
        } else if (strcmp(key, "process_pool_size") == 0) {
            g_config.process_pool_size = atoi(value);
        } else if (strcmp(key, "token_cps") == 0) {
            g_config.token_cps = atoi(value);
        } else if (strcmp(key, "token_burst") == 0) {
            g_config.token_burst = atoi(value);
        } else if (strcmp(key, "auth_token") == 0) {
            strncpy(g_config.auth_token, value, sizeof(g_config.auth_token) - 1);
        } else if (strcmp(key, "api_key") == 0) {
            strncpy(g_config.api_key, value, sizeof(g_config.api_key) - 1);
        } else if (strcmp(key, "cloud_server") == 0) {
            strncpy(g_config.cloud_server, value, sizeof(g_config.cloud_server) - 1);
        } else if (strcmp(key, "cloud_port") == 0) {
            g_config.cloud_port = atoi(value);
        }
    }
    
    fclose(fp);
    
    printf("[CONFIG] Configuration loaded from: %s\n", config_file);
    
    return 0;
}

/*
 * 功能：打印当前配置
 * 参数：无
 * 返回：无
 */
void config_print(void)
{
    printf("\n========== Current Configuration ==========\n");
    printf("TCP Port:          %d\n", g_config.tcp_port);
    printf("UDP Port:          %d\n", g_config.udp_port);
    printf("Log Level:         %d\n", g_config.log_level);
    printf("Log Path:          %s\n", g_config.log_path);
    printf("Thread Pool Size:  %d\n", g_config.thread_pool_size);
    printf("Process Pool Size: %d\n", g_config.process_pool_size);
    printf("Token CPS:         %d\n", g_config.token_cps);
    printf("Token Burst:       %d\n", g_config.token_burst);
    printf("Auth Token:        %s\n", g_config.auth_token);
    printf("Cloud Server:      %s:%d\n", g_config.cloud_server, g_config.cloud_port);
    printf("===========================================\n\n");
}
