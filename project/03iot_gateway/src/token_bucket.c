/*===============================================
 *   文件名称：token_bucket.c
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月15日
 *   描    述：令牌桶限流器
 *            实现流量控制、防止突发流量冲击
 ================================================*/

#include "../inc/iot_gateway.h"

// 全局令牌桶
token_bucket_t *g_token_bucket = NULL;

/*
 * 功能：创建令牌桶
 * 参数：cps - 每秒产生的令牌数
 *       burst - 令牌桶容量
 * 返回：令牌桶指针，失败返回NULL
 */
token_bucket_t *token_bucket_create(int cps, int burst)
{
    token_bucket_t *tb;
    
    if (cps <= 0 || burst <= 0) {
        fprintf(stderr, "[ERROR] Invalid token bucket parameters\n");
        return NULL;
    }
    
    // 分配内存
    tb = (token_bucket_t *)malloc(sizeof(token_bucket_t));
    if (tb == NULL) {
        perror("malloc");
        return NULL;
    }
    
    // 初始化参数
    tb->cps = cps;
    tb->burst = burst;
    tb->token = 0;  // 初始令牌数为0
    
    // 初始化互斥锁
    pthread_mutex_init(&tb->lock, NULL);
    
    log_write(LOG_INFO, "Token bucket created: cps=%d, burst=%d", cps, burst);
    
    return tb;
}

/*
 * 功能：尝试从令牌桶获取令牌（非阻塞）
 * 参数：tb - 令牌桶指针
 *       n - 需要的令牌数
 * 返回：实际获取的令牌数
 */
int token_bucket_fetch(token_bucket_t *tb, int n)
{
    int fetched = 0;
    
    if (tb == NULL || n <= 0) {
        return 0;
    }
    
    pthread_mutex_lock(&tb->lock);
    
    // 如果令牌充足，取走指定数量
    if (tb->token >= n) {
        fetched = n;
    } else {
        // 令牌不足，有多少取多少
        fetched = tb->token;
    }
    
    tb->token -= fetched;
    
    pthread_mutex_unlock(&tb->lock);
    
    return fetched;
}

/*
 * 功能：向令牌桶补充令牌（由定时器调用）
 * 参数：无
 * 返回：无
 */
void token_bucket_refill(void)
{
    if (g_token_bucket == NULL) {
        return;
    }
    
    pthread_mutex_lock(&g_token_bucket->lock);
    
    // 增加令牌
    g_token_bucket->token += g_token_bucket->cps;
    
    // 不能超过容量上限
    if (g_token_bucket->token > g_token_bucket->burst) {
        g_token_bucket->token = g_token_bucket->burst;
    }
    
    pthread_mutex_unlock(&g_token_bucket->lock);
    
    log_write(LOG_DEBUG, "Token bucket refilled: current=%d", g_token_bucket->token);
}

/*
 * 功能：销毁令牌桶
 * 参数：tb - 令牌桶指针
 * 返回：无
 */
void token_bucket_destroy(token_bucket_t *tb)
{
    if (tb == NULL) {
        return;
    }
    
    pthread_mutex_destroy(&tb->lock);
    free(tb);
    
    log_write(LOG_INFO, "Token bucket destroyed");
}
