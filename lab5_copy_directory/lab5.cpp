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

    int newfd;

    char buf[MAX_PATH];
    char command[MAX_PATH];
    int n;
    if(S_ISLNK(statbuf.st_mode)){
        sprintf(command,"touch -h -r %s %s",old_filename,new_filename); 

        n=readlink(old_filename,buf,MAX_PATH);
        buf[n]='\0';

        symlink(buf,new_filename);
    }
    else {
        sprintf(command,"touch -r %s %s",old_filename,new_filename); 

        newfd=creat(new_filename,statbuf.st_mode);
        int oldfd=open(old_filename,O_RDONLY);   
        while((n=read(oldfd,buf,MAX_PATH))>0)
            write(newfd,buf,n);
        close(oldfd);
    }
    close(newfd);
    
    system(command);
}

int CopyDir(char *oldpath, char *newpath){
    struct stat statbuf;
    struct dirent *entry;
    DIR *dp;

    if(chdir(newpath)==-1){
        lstat(oldpath,&statbuf);
        mkdir(newpath,statbuf.st_mode);
    }

    if((dp=opendir(oldpath))==NULL)
        perror("open error\n");
    chdir(oldpath);

    char next_oldpath[MAX_PATH], next_newpath[MAX_PATH];
    while((entry=readdir(dp))!=NULL){
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
    closedir(dp);

    char command[MAX_PATH];
    sprintf(command,"touch -r %s %s",oldpath, newpath); 
    system(command);

    chdir("../");
}

int main(int argc,char **argv){
    char rootdir[MAX_PATH],olddir[MAX_PATH],newdir[MAX_PATH];
    getcwd(rootdir,MAX_PATH);
    printf("rootdir %s\n",rootdir);

    chdir(argv[1]);
    getcwd(olddir,MAX_PATH);
    printf("olddir %s\n",olddir);

    chdir(rootdir);

    if(chdir(argv[2])==-1){
        struct stat stat_buf;
        lstat(olddir,&stat_buf);
        mkdir(argv[2],stat_buf.st_mode);
    }
    chdir(argv[2]);
    getcwd(newdir,MAX_PATH);
    printf("newdir %s\n",newdir);

    CopyDir(olddir,newdir);

    return 0;
}