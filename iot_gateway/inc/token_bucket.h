#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include "iot_gateway.h"
#include <pthread.h>
#include <stdint.h>

// 令牌桶控制块
typedef struct {
    long capacity;       // 桶的最大容量（防突发）
    long tokens;         // 当前可用令牌数
    long rate;           // 令牌补充速率（个/秒）
    uint64_t last_time;  // 上次计算的时间戳（毫秒）
    pthread_mutex_t lock; // 线程安全锁
} TokenBucket;

// 创建令牌桶
TokenBucket* token_bucket_create(long capacity, long rate);

// 销毁令牌桶
void token_bucket_destroy(TokenBucket *tb);

// 消费令牌 (返回 1 表示成功放行，返回 0 表示被限流拦截)
int token_bucket_consume(TokenBucket *tb, long count);

#endif // TOKEN_BUCKET_H