#include <stdio.h>

int main(){
    FILE *fp = fopen("test.txt", "w");
    if(fp == NULL){
        perror("Failed to open file");
        return -1;
    }
    char buf[] = "Hello, World!";
    if(fputs(buf, fp) == EOF){
        perror("Failed to write string");
        fclose(fp);
        return -2;
    }

    return 0;
}