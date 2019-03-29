#include<stdlib.h>
#include<unistd.h>

int main(int argv,char** argc){
    int time;
    if(argv>1)
        time=atoi(argc[1]);
    else 
        time = 2;
    
    printf("child process will sleep %ds",time);
    sleep(time);

    return 0;
}
