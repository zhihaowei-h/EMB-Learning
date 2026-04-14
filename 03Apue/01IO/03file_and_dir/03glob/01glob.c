#include <stdio.h>
#include <glob.h>
  
int main() {
    glob_t result;
    int ret;
      
    // 查找当前目录下所有 .c 文件
    ret = glob("*.c", 0, NULL, &result);
      
    if(ret == 0) {
        printf("找到 %zu 个 .c 文件：\n", result.gl_pathc);
        for(size_t i = 0; i < result.gl_pathc; i++) {
            printf("[%zu] %s\n", i, result.gl_pathv[i]);
        }
    } else {
        printf("匹配失败，错误码：%d\n", ret);
    }
      
    // 释放内存
    globfree(&result);
      
    return 0;
}