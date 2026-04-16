#include <stdio.h>
#include <glob.h>
#include <string.h>
  
void search_files(const char *pattern) {
    glob_t result;
    int flags = GLOB_BRACE | GLOB_TILDE | GLOB_NOSORT;
      
    int ret = glob(pattern, flags, NULL, &result);
    
    // 如果 匹配到 或者 没有匹配到
    if(ret == 0 || ret == GLOB_NOMATCH) {
        for(size_t i = 0; i < result.gl_pathc; i++) {
            printf("%s\n", result.gl_pathv[i]);
        }
          
        // 如果想递归，需要自己实现：
        // 对每个匹配到的目录，再次调用 glob
    }
      
    globfree(&result);
}
  
int main() {
    printf("当前目录所有 .c 文件：\n");
    search_files("*.c");
      
    printf("\n当前目录所有 .h 和 .c 文件：\n");
    search_files("*.{c,h}");
      
    printf("\n/etc 下所有 .conf 文件：\n");
    search_files("/etc/*.conf");
      
    printf("\n家目录 下所有 .txt 文件：\n");
    search_files("~/*.txt");
      
    return 0;
}