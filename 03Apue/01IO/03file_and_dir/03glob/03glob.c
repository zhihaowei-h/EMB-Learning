#include <stdio.h>
#include <glob.h>
#include <string.h>
  
// 自定义错误处理函数
int my_errfunc(const char *epath, int eerrno) {
    fprintf(stderr, "访问 %s 时出错：%s\n", epath, strerror(eerrno));
    return 0;  // 返回 0 继续匹配，返回非 0 停止
}
  
int main() {
    glob_t result;
      
    int ret = glob("*.h", 0, my_errfunc, &result);  // 假如在一个权限为000的文件夹中执行该程序，就会报错转而去执行my_errfunc
      
    switch(ret) {
        case 0:
            printf("成功，找到 %zu 个\n", result.gl_pathc);
            break;
        case GLOB_NOMATCH:
            printf("没有匹配的文件\n");
            break;
        case GLOB_ABORTED:
            printf("读取目录时出错\n");
            break;
        case GLOB_NOSPACE:
            printf("内存不足\n");
            break;
    }
      
    globfree(&result);
    return 0;
}