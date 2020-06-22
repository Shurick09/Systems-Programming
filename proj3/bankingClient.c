#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "banking.h"

//sends valid input to server
void commandInputFunc(void* args){
    int socketNum = *(int *)args;
    char str[1500];
    //sleep for 1 second so the prompt prints after the connection message
    sleep(1);

    while(1){
        write(STDOUT, "Enter a command:", 16);
        int n = read(STDIN,&str,1500);
        str[n - 1] = '\0';
	//if valid command, send input to server
	if(strncmp("create", str, 6) == 0 || strncmp("serve", str, 5) == 0 || strncmp("deposit", str, 7) == 0 || strncmp("withdraw", str, 8) == 0 || strcmp("query", str) == 0 || strcmp("end", str) == 0){
         
            send(socketNum,str,strlen(str),0);
	    sleep(2);
	}
	//if client is quitting, send quit command and exit thread
        else if(strcmp(str,"quit") == 0){
	    send(socketNum, str, strlen(str), 0);
	    break;
        }
	else{
            write(STDOUT, "Command not found\n", 18);
	}

        bzero(str, strlen(str)); 
    }
}
//outputs and interprets server messages
void responseOutputFunc(void* args){
    int socketNum = *(int *)args;
    char* buffer = (char*) malloc(sizeof(char)*300);
    int var = 0;
    int tot = 0;
    //int isRunning = 1;
    //int infoFromServer = 0;
    //ssize_t total = 0;
    while(1){
        memset(buffer,'\0',300);
        var = recv(socketNum,buffer,300,0);
	if(strcmp(buffer, "Server exited") == 0){
	    write(STDOUT, "Server terminated. Client exiting...\n", 37);
            exit(0);
	}
	else if(strcmp(buffer, "Client exiting")== 0){
            write(STDOUT, "Client exiting...\n", 18);
	    close(socketNum);
	    exit(0);
	}
         
        if(buffer[1] != '\0'){
	    
            sprintf(buffer, "%s\n",buffer);
	    write(STDOUT, buffer,strlen(buffer));
        }
    }
}

int main(int argc, char *argv[]){

    if(argc != 3){
        write(STDOUT, "Error: Incorrect amount of arguments", 39);
        return 0;
    }
     
    int socketNum;
    struct sockaddr_in dest; 
    //get socket number
    socketNum = socket(AF_INET, SOCK_STREAM, 0);
    //zero out destination struct
    bzero(&dest, sizeof(dest));               
    //get data from domain name
    dest.sin_family = AF_INET;
    struct hostent *hostStruct = gethostbyname(argv[1]);
    //set IP and port number
    dest.sin_addr.s_addr = inet_addr(inet_ntoa( *( struct in_addr*)( hostStruct->h_addr)));  
    dest.sin_port = htons(atoi(argv[2]));  
    //connect to server
    
    while(connect(socketNum, (struct sockaddr *)&dest, sizeof(struct sockaddr_in)) < 0){
        printf("Connecting to server...\n");
	sleep(1);
    }
    /*
    struct sockaddr *addr;
    int socketFD = accept(socketNum, addr, NULL);
    recieve data and put into buffer
    char buffer[500]; 
    int bufferLen = recv(socketNum, buffer, 500, 0);
    buffer[bufferLen] = '\0';
    printf("Received %s (%d bytes).\n", buffer, bufferLen);

    char command[500] = "create accountName";
    scanf("%d", &command);
    send(socketFD, command, strlen(command), 0); 
    int x = 0;
    if(x = read(socketNum, recv
    close(socketNum);
    return EXIT_SUCCESS;
    */
    pthread_t commandInput;
    pthread_t responseOutput;
    
    pthread_create(&commandInput,0,commandInputFunc,&socketNum);
    pthread_create(&responseOutput,0,responseOutputFunc,&socketNum);

    pthread_join(responseOutput,NULL);
    pthread_cancel(commandInput);

    close(socketNum);

    return 0;
}
