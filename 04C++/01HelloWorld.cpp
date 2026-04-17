#include <iostream> // C++的输入输出的头文件(其中包含了cout和cin等输入输出对象的定义)
using namespace std; // 使用std命名空间，cout和endl等对象的名称都属于std命名空间，这样后，不需要每次都写std::cout或std::endl

int main(void){
    cout << "Hello C++!" << endl;
    printf("Hello C!\n"); // printf是C语言的输出函数，C++中也可以使用，最后包含<cstdio>头文件(但是不包含也可以使用, 因为iostream头文件中已经包含了cstdio头文件)，但是在C++中更推荐使用cout进行输出，因为它更安全、更灵活、更易于使用。
    return 0;
}
