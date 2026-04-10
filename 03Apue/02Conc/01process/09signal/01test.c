#include <stdio.h>
#include <unistd.h>

int main(void)
{
    while(1)//死循环,每1s往标准输出写一个"*"
    {   
        write(1, "*", 1);//往标准输出写一个"*"
        sleep(1);//睡1s
    }
    
    return 0;
}