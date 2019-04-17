#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

int main(int argv, char **argc)
{
    int pid;

    struct timeval tv_start, tv_end;
    if (vfork())    //parent process
    {
        //wait child process exec

        printf("start process\n");
        gettimeofday(&tv_start, NULL);
        wait(pid);  //wait child process finish
        gettimeofday(&tv_end, NULL);

        long int sec = tv_end.tv_sec - tv_start.tv_sec;
        long int usec = tv_end.tv_usec - tv_start.tv_usec;
        printf("runtime: %lds %ldmsec %ldusec\n", sec, usec / 1000, usec % 1000);
    }
    else    //child process
    {
        //child process exec, and parent process begin
        execlp(argc[1], argc[1], argc[2], 0);
        exit(0);
    }
    return 0;
}

