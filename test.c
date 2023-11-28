#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<signal.h>
#include<sys/types.h>

void    handler(int signum)
{
    printf("%d done\n", signum);
}

int main(void)
{
    int pip[2];

    // signal(SIGPIPE, handler);
    pipe(pip);
    if (fork() == 0)
    {
        close(pip[0]);
        write(pip[1], "A", 1);
        close(pip[1]);
        // write(pip[1], "A", 1);
        // write(pip[1], "A", 1);
        // write(pip[1], "B", 1);
        printf("done");
    }
    else
    {
        char    ch;
        int     ret;

        close(pip[1]);
        ret = read(pip[0], &ch, 1);
        printf("[%c %d]\n", ch, ret);
        ret = read(pip[0], &ch, 1);
        printf("[%c %d]\n", ch, ret);
        ret = read(pip[0], &ch, 1);
        printf("[%c %d]\n", ch, ret);
        wait(NULL);
        printf("parent");
    }
}