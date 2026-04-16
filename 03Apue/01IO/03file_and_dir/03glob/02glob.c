// 同时匹配 .c 和 .h 文件
#include <stdio.h>
#include <glob.h>
  
int main() {
    glob_t result;
      
    int ret = glob("*.c", 0, NULL, &result);
  
    // 不管配没匹配到.c文件，都接着尝试匹配.h文件，追加到result中
    if(ret == 0 || ret == GLOB_NOMATCH) {
        ret = glob("*.h", GLOB_APPEND, NULL, &result);
    }
      
    printf("找到 %zu 个文件：\n", result.gl_pathc);
    for(size_t i = 0; i < result.gl_pathc; i++) {
        printf("%s\n", result.gl_pathv[i]);
    }
      
    globfree(&result);
    return 0;
}