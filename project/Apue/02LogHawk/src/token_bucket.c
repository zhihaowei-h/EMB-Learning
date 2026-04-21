/*
 * token_bucket.c —— 令牌桶限流
 *
 * 原理：
 *   想象一个桶，系统每秒往里扔 cps 个令牌，桶最多装 burst 个
 *   每次要写一条日志，先从桶里拿一个令牌
 *   拿到了才能写，拿不到（桶空了）就丢弃或等待
 *
 * 作用：防止日志爆发时把磁盘或下游系统压垮
 */

#include "../inc/loghawk.h"

/* 初始化令牌桶 */
void tb_init(struct token_bucket *tb, int cps, int burst) {
    tb->tokens      = burst;      /* 初始满桶 */
    tb->cps         = cps;
    tb->burst       = burst;
    tb->last_refill = time(NULL);
}

/*
 * 尝试消耗一个令牌
 * 返回 1：成功（可以写日志）
 * 返回 0：桶空，限流（丢弃这条）
 */
int tb_consume(struct token_bucket *tb) {
    /* 根据时间差，补充应该生成的令牌 */
    time_t now = time(NULL);
    int elapsed = (int)(now - tb->last_refill);
    if (elapsed > 0) {
        tb->tokens += elapsed * tb->cps;
        if (tb->tokens > tb->burst)
            tb->tokens = tb->burst;   /* 不超过桶容量 */
        tb->last_refill = now;
    }

    /* 消耗一个令牌 */
    if (tb->tokens > 0) {
        tb->tokens--;
        return 1;   /* 通过 */
    }
    return 0;       /* 被限流 */
}
