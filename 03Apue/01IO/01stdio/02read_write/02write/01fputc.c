#include <stdio.h>

int main(){
    FILE *fp = fopen("test.txt", "w");
    if(fp == NULL){
        perror("Failed to open file");
        return -1;
    }

    int c = 'A';
    if(fputc(c, fp) == EOF){
        perror("Failed to write character");
        fclose(fp);
        return -2;
    }

    return 0;
}