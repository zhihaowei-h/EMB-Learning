#include <stdio.h>
#include <unistd.h>

int main() {
    printf("【调用者】准备发射，启动可执行文件[./target] ...\n");

    // 注意这里：
    // 第1个参数 "./target" 是硬盘上的真实文件路径
    // 第2个参数 "hello" 是我们赋予它的虚假名字 (传给目标程序的 argv[0])
    
    execl("./target", "hello", (char *)NULL);

    // 如果 exec 成功，下面的代码不会执行
    perror("exec 失败");
    return 1;
}