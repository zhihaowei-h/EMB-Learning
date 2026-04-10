#pragma once

// 定义一些宏，规定消息队列的键值和权限等信息
#define PATH "/etc/passwd" // 定义一个宏，规定要操作的文件路径，这里是系统的密码文件
#define PROJ_ID 'a'        // 定义一个宏，规定消息队列的项目标识符(pro_id)，这里是字符'a'
#define STRSIZE 1024       // 定义一个宏，规定消息字符串的大小，这里是128字节

// 定义一个结构体，表示消息队列中的消息格式
struct msg_st{
    long mtype;         // 官方规定需要有一个msgtyp成员，类型必须是long，用来标识消息的类型，可以根据这个类型来区分不同的消息
    char str[STRSIZE];  // 定义一个字符数组，用来存储每条消息的内容，大小由STRSIZE宏定义，这里是1024字节
};