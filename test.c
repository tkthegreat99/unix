#include"unistd.h"
#include"sys/types.h"
#include"sys/fcntl.h"
#include<stdio.h>

int main(void)
{
    int fd;

    fd = open("I\/O\ node\ 0", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    printf("fd = %d\n", fd);
}