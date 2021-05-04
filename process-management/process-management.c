#include <fcntl.h>  
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <stdlib.h>

int numberOfWords(char buffer[]);//kelime sayisini bulur
void process(char prog[],char **args);//child process olusturur

int main() {

    int fd = open("programs.txt", O_RDONLY);
    if(-1 == fd){
       perror("cannot open file: ");
       return -1;
    }
    char BUFFER[256];
    ssize_t rd = read(fd,BUFFER,256);
    if(-1 == rd){
        perror("cannot read file: ");
        return -1;
    }
    //nword: kelime sayisi
    int nword = numberOfWords(BUFFER);
    
    char **args = (char **)malloc(nword * sizeof(char*));
    if (args == NULL) {
        printf("Memory not allocated.\n");
        exit(0);
    }
    memset(args, 0, sizeof(char*));
    printf("%d",args);
    char *word = strtok(BUFFER," ");
    
    int i=0;
    while(word != NULL && i<nword){
        args[i]=malloc( (strlen(word)+1) *sizeof(char));
        strncpy(args[i],word,strlen(word));
        word = strtok(NULL," ");
        ++i;
    }
    args[nword]= NULL;
    char prog[50] ="/usr/bin/";
    strcat(prog, args[0]);

    process(prog,args);
    
    free(args);
    close(fd);
    
   return 0;
}
int numberOfWords(char buffer[]){
    char temp[256];
    strncpy(temp,buffer,256);
    int nword = 0;
    char *str = strtok(temp," ");
    
    while ( str != NULL ){
        nword++;
        str = strtok(NULL," ");
    }
    return nword;
}
void process(char prog[],char **args){

    int ret ;
    pid_t id = fork();
    if(0 == id){
        ret = execv(prog, args);
        if(-1 == ret){
            perror("execv");
        }
        printf("process id: %d program adi: %s",
        id,args[0]);
    }
    else {
        printf("parent process,process id: %d",id);
    }
}