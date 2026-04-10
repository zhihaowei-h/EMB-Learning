/*使用系统调用IO函数实现mycat*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 128

static int mycat(int fd){
    char buf[BUFSIZE] = {0}; // 存储读取到的数据
    int count = 0; // 存储成功读取到的字节数

    while(1){
        memset(buf, 0, BUFSIZE); // 清空脏数据
        count = read(fd, buf, BUFSIZE); // 从文件中读取数据，1次读取BUFSIZE字节，存储到buf中，返回成功读取到的字节数

        // 如果读到了文件结尾的位置
        if(count == 0){
            break;//跳出死循环
        }else if(count < 0){// 如果读取失败
            perror("read()");// 打印错误信息
            return -1; // 由于读取失败,结束函数,并且返回-1
        }
        write(1, buf, count);// 把数据写入到标准输出文件中
    }
    
    return 0;
}

int main(int argc, char *argv[]){
    int fd = 0; // fd变量用来保存打开文件的文件描述符
    int ret = 0; // ret变量用来保存mycat函数的返回值

    // 如果命令行参数个数少于2个
    if(argc < 2){
        fprintf(stderr, "Usage : %s + filename\n", argv[0]);//打印使用说明
        ret = -1;
        goto ERR_ARGC_1;
    }

    fd = open(argv[1], O_RDONLY); // 通过open(2)以只读的形式打开文件
    // 如果打开文件是否失败
    if(fd < 0){
        perror("open()");//打印错误信息
        ret = -2;
        goto ERR_OPEN_2;
    }

    ret = mycat(fd); // 调用自己实现的mycat()
    close(fd); // 通过close(2)关闭文件
    return 0; // 正常结束程序,返回0

ERR_OPEN_2:
ERR_ARGC_1:
    return ret;//返回ret变量的值
}