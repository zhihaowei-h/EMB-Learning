#include "token_bucket.h"
#include <time.h>

// 获取当前系统的单调时间（毫秒精度）
static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

TokenBucket* token_bucket_create(long capacity, long rate) {
    TokenBucket *tb = (TokenBucket *)malloc(sizeof(TokenBucket));
    if (!tb) return NULL;

    tb->capacity = capacity;
    tb->tokens = capacity; // 初始状态桶是满的
    tb->rate = rate;
    tb->last_time = get_current_time_ms();
    
    pthread_mutex_init(&tb->lock, NULL);
    
    LOG_INFO("Token bucket created: Capacity=%ld, Rate=%ld/s", capacity, rate);
    return tb;
}

void token_bucket_destroy(TokenBucket *tb) {
    if (tb) {
        pthread_mutex_destroy(&tb->lock);
        free(tb);
    }
}

// 核心限流算法（懒加载补充法）
int token_bucket_consume(TokenBucket *tb, long count) {
    if (!tb || count <= 0) return 0;

    pthread_mutex_lock(&tb->lock);

    uint64_t now = get_current_time_ms();
    uint64_t elapsed_ms = now - tb->last_time;

    // 1. 根据距离上次计算流逝的时间，实时补充令牌
    long generated_tokens = (elapsed_ms * tb->rate) / 1000;
    if (generated_tokens > 0) {
        tb->tokens += generated_tokens;
        if (tb->tokens > tb->capacity) {
            tb->tokens = tb->capacity; // 不能超过桶的上限
        }
        tb->last_time = now; // 只有真实产生了令牌才更新时间，防止高频小碎步请求导致精度丢失
    }

    // 2. 扣减令牌
    int success = 0;
    if (tb->tokens >= count) {
        tb->tokens -= count;
        success = 1; // 成功拿到令牌，放行
    }

    pthread_mutex_unlock(&tb->lock);
    return success;
}