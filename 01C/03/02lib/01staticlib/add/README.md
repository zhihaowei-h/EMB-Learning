目标: 尝试将add.c封装成静态库文件

[1]翻译: 将人类能看懂的add.c翻译成机器能看懂的二进制目标文件add.o
gcc -c add.c -o add.o // 将add.c变成add.o



[2]打包: 使用ar工具把一个或多个.o文件(这里只有1个add.o)塞进一个静态库文件里
ar -rcs libadd.a add.o // 将add.o塞进静态库文件libadd.a中，并给该静态库文件生成索引
(这时，add.o就没有作用了，可以删除)



[3]执行: gcc main.c -L. -ladd -o myapp  // [-L（指定库路径）和 -l（指定库名）]
当你执行 gcc main.c -L. -ladd -o myapp 时，gcc 在后台悄悄做了两件事：
    临时编译：它先将 main.c 编译成一个临时的 main.o（存放在内存或临时文件夹中，执行完就删了）。
    正式链接：它把这个临时的 main.o 与当前路径下的 libadd.a 进行链接，最终输出 myapp。

(到这里，静态库文件就已经建立完毕了，但是还可以优化)



[4]把制作好的静态库文件拷贝到/lib下: sudo mv libadd.a /lib

...