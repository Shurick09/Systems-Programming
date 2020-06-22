#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "banking.h"

pthread_mutex_t mut;

boolean isAccessing = FALSE;
boolean caughtInterupt = FALSE;
LLofFD* activeFD;

account* bankAccountsLL = NULL;

LLofFD* addFD(LLofFD* activeFD,int fd){
    LLofFD* node = (LLofFD*)malloc(sizeof(LLofFD));
    node->fd = fd;
    node->next = NULL;
    if(activeFD == NULL){
        activeFD = node;
        return activeFD;
    }
    LLofFD* ptr = activeFD;
    
    while(ptr->next != NULL){
        ptr = ptr->next;
    }
    ptr->next = node;
    return activeFD;
}

//searches linked list and returns pointer to account with the given name
account* search(char* searchName){
   account* ptr = bankAccountsLL;
   while(ptr != NULL){
       if(strcmp(ptr->accountName,searchName) == 0)
           return ptr;
       ptr = ptr->next;
   }
   return NULL;
}

void addAccount(char* name){
    //make a new node
    account* newAccount = (account*)malloc(sizeof(account));
    newAccount->accountName = (char*)malloc(sizeof(char)*255); 
    strcpy(newAccount->accountName, name);
    newAccount->inSession = FALSE;
    newAccount->currBalance = 0.0;
    //point to front of list and set front to created node
    newAccount->next = bankAccountsLL;
    bankAccountsLL = newAccount;
}
//returns account name if it is being served, and NULL otherwise
char* communicateBankAccounts(char* command,char* accountName, int socketNum){
    char* userArg = (char*)malloc(sizeof(char)*255);
    account* nodePtr; 

    //if acountName is null, no account is being served
    //otherwise accountName contains the current account
    if(accountName != NULL){
	nodePtr = search(accountName);
	if(nodePtr == NULL){
            //write(STDERR && socketNum,"Error:account not found",23);
            send(socketNum,"Error: account not found",23,0);
	}
    }
    //check which command is being recieved
    if(strncmp(command,"create",6) == 0){

        //get argument from command
        strcpy(userArg,&command[7]);
        
	if(accountName != NULL){
           //write(STDERR && socketNum,"Error:Can't create account while in another account",53);
           send(socketNum,"Error:Can't create account while in another account",53,0);
        }
        else if(search(userArg) != NULL){
            //write(STDERR && socketNum,"Error: Account already exists",30);
            send(socketNum,"Error: Account already exists",30,0);
        }
        else{
            addAccount(userArg);
            char* buffer = "Account created\n";
            send(socketNum,"Account created",strlen(buffer),0);
            //write(socketNum,buffer,15);
	    return NULL;
        }
    }    
    else if(strncmp(command,"serve",5) == 0){
        strcpy(userArg,&command[6]);
	nodePtr = search(userArg);
        
	if(accountName != NULL){
            //write(STDERR && socketNum,"Error:Can't serve account while in another account",51);
            send(socketNum,"Error:Can't serve account while in another account",51,0);
        }
	else if(nodePtr == NULL){
	    send(socketNum, "Error: Account not found", 25, 0);
	}
	else if(nodePtr != NULL && nodePtr->inSession == TRUE){
            send(socketNum, "Error: Account in session", 25, 0);
	}
        else{
            send(socketNum,"In service",11,0);
            //write(STDOUT && socketNum,"In service\n",12);
            strcpy(userArg,&command[6]);
	    accountName = userArg;
	    nodePtr = search(userArg);
            nodePtr->inSession = TRUE;
        }
    }
    else if(strncmp(command,"deposit",7) == 0){
        if(accountName == NULL){
            //write(STDERR && socketNum,"Error: Can't deposit without an account",51);
            send(socketNum,"Error: Can't deposit without an account",51,0);
        }
        else{
            //write(STDOUT && socketNum,"Deposit completed\n",19);
            send(socketNum,"Deposit completed",18,0);
            strcpy(userArg, &command[8]);
            double amount = atof(userArg);
	    nodePtr->currBalance = nodePtr->currBalance + amount;
        }
    }
    else if(strncmp(command,"withdraw",8) == 0){
        if(accountName == NULL){
            //write(STDERR && socketNum,"Error: Can't withdraw without an account",51);
            send(socketNum,"Error: Can't withdraw without an account",51,0);
        }
	else if(nodePtr->currBalance < atof(&command[9])){
	    send(socketNum, "Error: Balance cannot go negative",33, 0);
	}
        else{
            //write(STDOUT && socketNum,"Withdraw completed",18);
            send(socketNum,"Withdraw completed",18,0);
            strcpy(userArg, &command[9]);
            double amount = atof(userArg);
            nodePtr->currBalance = nodePtr->currBalance - amount;
        }
    }
    else if(strncmp(command,"query", 5) == 0){
        if(accountName == NULL){
            //write(STDERR && socketNum,"Error: Can't query without an account",51);
            send(socketNum,"Error: Can't query without an account",51,0);
        }
        else{
	    char balanceToSend[500];
	    sprintf(balanceToSend, "$%.2f", nodePtr->currBalance);
	    send(socketNum, balanceToSend, strlen(balanceToSend), 0);
	    //write(socketNum,balanceToSend,strlen(balanceToSend));
        }
    }
    else if(strncmp(command,"end", 3) == 0){
        if(accountName == NULL){
            //write(STDERR && socketNum,"Error: Can't exit without a session",51);
            send(socketNum,"Error: Can't exit without a session",51,0);
        }
	else{
            //write(STDOUT && socketNum,"Account exited",14);
            send(socketNum,"Session ended", 13,0);
	    nodePtr->inSession = FALSE;
	    accountName = NULL;
            //nodePtr = bankAccountsLL;
	    return NULL; 
        }
    }
    else{
        //write(STDERR && socketNum,"Command not found",18);
        send(socketNum,"Command not found",18,0);
    }
    return accountName;
}

//recieves commands from clients
void clientServerCommunication(void* args){
    int fd = *(int *)args;
    char buffer[1500];
    char* currAccount = NULL;
    int var = 0;
    boolean isRunning = TRUE;
    while(isRunning == TRUE){
        /*
	if(caughtInterupt == TRUE){
	    isRunning = FALSE;
	    send(fd, "Server exited", 15, 0);
	    exit(0);
	}
        */
	var = recv(fd,buffer,1500,0);
     
	//receives error
        if(var == -1){
            printf("something went wrong\n");
	    isRunning = FALSE;
            break;
        }
	//check data from server and perform command
        buffer[var] = '\0';    
	if(strncmp(buffer, "quit", 4) == 0){
	    printf("Disconnecting from client\n");
	    if(currAccount != NULL){
                communicateBankAccounts("end", currAccount, fd);
	    }
            send(fd,"Client exiting",14,0);
            isRunning = FALSE;
        }
        else{
	    if(currAccount){
                //printf("Currently serving:%s\n", currAccount);
            }
	    //wait while data is being accessed
	    while(isAccessing == TRUE){};
            isAccessing = TRUE;
            pthread_mutex_lock(&mut);
            currAccount = communicateBankAccounts(buffer, currAccount, fd);
            pthread_mutex_unlock(&mut);
	    isAccessing = FALSE;
            
	}
	sleep(1);
    }    
    close(fd);
}

//accepts connections and spawns a thread for each client
void sessionAcceptor(void* args){
    char* port = (char*)args;
    struct sockaddr_in serv; 
    int socketNum; 
    socketNum = socket(AF_INET, SOCK_STREAM, 0);    
    //zero out server info struct and set type to TCP
    bzero(&serv, sizeof(serv));
    //set server address and port number
    serv.sin_family = AF_INET; 
    serv.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv.sin_port = htons(atoi(port));
    //bind information to socket
    bind(socketNum, (struct sockaddr *)&serv, sizeof(struct sockaddr));

    //listen for connection, up to 1 connection
    listen(socketNum, 1);

    //pthread_t clientThreads[500];
    //int clientCount = 0;
    //accept new clients
    while(caughtInterupt == FALSE){
        
	//printf("listening...");
        listen(socketNum,1);
	//printf("listened");
 
        int newFd = accept(socketNum,(struct sockaddr*)NULL, NULL);
        printf("Successfully connected to client.\n");
        write(newFd,"Successfully connected to server.",34);
        
        activeFD = addFD(activeFD,newFd);

        pthread_t clientServerCom;
        pthread_create(&clientServerCom,0,clientServerCommunication,&newFd);
    }
    //wait for child threads
    //int i;
    //for(i = 0; i < clientCount; i++){
    //    pthread_join(clientThreads[i], NULL);
    //}
    printf("Exiting connection acceptor\n");
    close(socketNum);
}
//print out data from list
//invoked by SIGALRM every 15 seconds
void timedOutput(){
    //wait until other threads are not accessing data
    //printf("%d\n",isAccessing);
    while(isAccessing == TRUE){};
    isAccessing = TRUE;
    write(STDOUT, "\nDiagnostic Output:\n", 20);

    account* ptr = bankAccountsLL;
    if(ptr == NULL)
        write(STDOUT, "List is empty\n", 14);
    while(ptr != NULL){
        char buffer[350];
	int n = sprintf(buffer, "%s\t$%.2f\t", ptr->accountName, ptr->currBalance);
	write(STDOUT, buffer, n);
	if(ptr->inSession == TRUE)
	    printf("IN SESSION");
	printf("\n");
        ptr = ptr->next;
    }
    isAccessing = FALSE;
}
void setInterupt(){
    write(STDOUT, "Caught user interupt. Exiting...\n", 34);
    caughtInterupt = TRUE;
    LLofFD* ptr = activeFD;
    while(ptr != NULL){
        send(ptr->fd,"Server exited",14,0);
        close(ptr->fd);
        ptr = ptr->next;
        
    }
    
    exit(0);
}
int main(int argc, char *argv[])
{
    if(argc != 2){
        write(STDOUT,"Error: Incorrect amount of arguments",37);
        return 0;
    }
    char* portNum = (char*)malloc(sizeof(char) * 20);
    strcpy(portNum,argv[1]);
    //create new thread to accept clients
    pthread_t saThread;
    void (*saPtr)(void*) = sessionAcceptor;
    pthread_create(&saThread, NULL, saPtr, (void *)portNum);
    //set up signal handler and timer
    signal(SIGALRM, timedOutput);
    struct itimerval it, oldIt;
    it.it_interval.tv_sec = 15;
    it.it_interval.tv_usec = 0;
    it.it_value = it.it_interval;
    if(setitimer(ITIMER_REAL, &it, &oldIt) == -1)
        write(STDERR, "error setting timer",19);
    //set up sighandler for sigint
    signal(SIGINT, setInterupt);
    
    //wait for signals
    while(caughtInterupt == FALSE)
        sleep(1);
    

    pthread_join(saThread, NULL);

    return EXIT_SUCCESS;
}
