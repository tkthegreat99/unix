#ifndef PROJECT_H
# define PROJECT_H

#include<unistd.h>
#include<errno.h>
#include<sys/fcntl.h>
#include<sys/types.h>
#include<stdio.h>
#include <stdlib.h>

typedef struct s_mytime
{
    struct timeval stime, etime;
	int time_result;
}   t_mytime;

#define MB 256000
#define IN 0
#define OUT 1

#endif