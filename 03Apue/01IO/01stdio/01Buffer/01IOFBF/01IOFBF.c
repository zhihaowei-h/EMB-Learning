#include <stdio.h>
#include <unistd.h>  // for sleep()
  
int main(void){
    FILE *fp = fopen("output.txt", "w"); // fopen
    // 如果打开文件失败
    if (fp == NULL) {
        perror("文件打开失败");
        return 1;
    }

    // 打开文件后，fp指向一个管理全缓冲区的FILE结构体，所有写入这个文件的数据都会先存储在缓冲区中

    // 写入数据到fp所指文件流的缓冲区
    fprintf(fp, "Hello World!"); // 将"Hello World!"写入fp的缓冲区
    printf("数据已写入缓冲区，但还未写入文件\n");
     
    // 暂停5秒，让你有时间去查看文件内容
    printf("等待5秒...\n");
    sleep(5);
     
    // 此时去查看output.txt，文件应该是空的
    printf("现在去查看output.txt，应该是空的\n");
    printf("按回车键继续..."); // 往stdout输出提示信息，stdout是行缓冲的，"按回车键继续..."进入stdout的缓冲区
    getchar(); // 尝试从标准输入中读取1个字符
      
    // 关闭文件，这会刷新fp所指文件流的缓冲区
    fclose(fp);  // fclose(3)内部有ffush(2),它会刷新指针fp所指向的结构体所管理的缓冲区; close(2)没有，fclose这个“f”就是这么来的
    printf("文件已关闭，缓冲区被刷新，现在去查看output.txt，应该能看到内容了\n");
      
    return 0;
}