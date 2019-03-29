#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>

#define MAX_PATH 255

int CopySingleFile(char *old_filename, char *new_filename){
    struct stat statbuf;
    lstat(old_filename,&statbuf);

    int oldfd,newfd;
    oldfd=open(old_filename,O_RDONLY);
    newfd=creat(new_filename,statbuf.st_mode);

    char buf[MAX_PATH];
    int n;
    while((n=read(oldfd,buf,MAX_PATH))>0)
        write(newfd,buf,n);
    
    
    system("touch -r %s %s",old_filename, new_filename);
    close(oldfd);
    close(newfd);
}

int CopyDir(char *oldpath, char *newpath){
    mkdir(newpath,0644);

    struct stat statbuf;
    struct dirent *entry;
    DIR *dp;

    if((dp==opendir(oldpath))==NULL)
        perror("open error\n");
    chdir(oldpath);

    char next_oldpath[MAX_PATH], next_newpath[MAX_PATH];
    while((entry==readdir(dp))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)
            continue;
        
        lstat(entry->d_name,&statbuf);

		sprintf(next_oldpath, "%s\/%s", oldpath, entry->d_name);
		sprintf(next_newpath, "%s\/%s", newpath, entry->d_name);

        if(S_ISDIR(statbuf.st_mode)){
            CopyDir(next_oldpath,next_newpath);
        }
        else{
            CopySingleFile(next_oldpath,next_newpath);
        }
    }
    chdir("..");
    closedir(dp);

}

int main(int argc,char **argv){


    return 0;
}