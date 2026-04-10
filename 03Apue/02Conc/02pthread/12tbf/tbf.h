#pragma once

#define PTHREAD   // 定义线程数量
#define CPS 10    // 定义每秒生成的令牌数
#define BURST 100 // 定义令牌桶的容量

int tbf_init(int cps, int burst);

int tbf_fetch_token(int td, int n);

int tbf_destroy(int td);




#if defined(PTHREAD)

/**
 * @brief 
 * 功能: 返还令牌
 * @param
 *  td: 令牌桶的下标 
 *  ntoken: 要返还的令牌数量
 * @return
 * 成功返还的令牌数量，失败返回-1 
 */
int tbf_return_token(int td, int ntoken);


/**
 * @brief 
 * 功能: 销毁所有令牌桶
 * @return
 * 无返回值
 */
void tbf_destroy_all(void);

#endif