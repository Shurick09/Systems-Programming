#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "scannerCSVsorter.h"
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

//global variable for child count
static int * FATAL_ERROR; //0 if no fatal error
static int * childCount;
//Takes quotes out of a token if it is the one being sorted by, so that quotes don't interfere with the comparison
char* cleanQuotes(char* token){
    char* noQuotes = (char*)malloc(sizeof(char) * 150);
    if(token[0] == '"'){
        int i;
        int c = 0;
        for(i = 1; i < strlen(token) - 1;i++){
            noQuotes[c] = token[i];
            c++;
        }
        strcpy(token,noQuotes);
    }
    return token;
}

//Trims leading and trailing white space from a token if it is the one being sorted by, so that the white space doesn't interfere with comparison
char* trimWhiteSpace(char* token){
    char *trail;
    while(isspace((unsigned char)*token)){
        token++;
    }
       if(*token == 0){
           return token;
        }
        trail = token + strlen(token) - 1;
        while(trail > token && isspace((unsigned char)*trail)){ 
            trail--;
        }
        trail[1] = '\0';
        return token;
}

//Creates a node using the movie meta data in buffer 
//Called for each movie, so that each row has it's own node in the linked list
node* create(char* buffer,int col,int totCol){
    if(*FATAL_ERROR == 1){
        exit(-1);
    }
    node* newNode = (node*)malloc(sizeof(node));
    if(newNode == NULL){
        printf("Error creating node\n");
        exit(0);
    }
    char** row = (char**)malloc(sizeof(char*) * totCol);
    int s;
    for(s = 0;s < totCol;s++){
        row[s] = (char*)malloc(sizeof(char) * 150);
    }
    //char* token = (char*)malloc(sizeof(char) * 150);
    char* token;
    char* sortBy = (char*)malloc(sizeof(char) * 150);
    token = strtok(buffer,",");//Breaks up the buffer into tokens to populate the linked list
    int i = 0; 
    int spec = 0;
    char* tempToken;
    while(i != totCol){

	if(token == NULL && i < totCol){
            write(STDERR, "Error: Invalid .csv file\n", 26);
            //*FATAL_ERROR = 1;
            exit(-1);
	}
        //reallocates sortBy and row strings if token is too long
        if(strlen(token) > 150){
            sortBy = realloc(sortBy, strlen(token)+2); 
            row[i] = realloc(row[i], strlen(token)+2); 
	}
        if(spec == 0){
            if(token[0] == '"'){
                spec = 1;
                tempToken = (char*)malloc(sizeof(char) * 150);
                strcpy(tempToken,token);
                strcat(tempToken,",");
            }
            else{
                strcpy(row[i],token);
                
                if(i == col){
                    strcpy(sortBy,trimWhiteSpace(token));
                }
                i++;
            }
        }
        else if(spec == 1){//If quote appears in a token
            if(token[strlen(token) - 1] == '"'){
                spec = 0;
                strcat(tempToken,token); 
                strcpy(row[i],tempToken);
                if(i == col){
                    strcpy(sortBy,cleanQuotes(trimWhiteSpace(tempToken)));
                }
                i++;
                free(tempToken);   
            }
            else{
                strcat(tempToken,token);
                strcat(tempToken,",");
            }
        }
        token = strtok(NULL,",");

    }
    newNode->row = row;
    newNode->sortBy = sortBy;
    newNode->next = NULL;
    free(token);
    return newNode;  
}    
         

//Adds a node to the linked list of movies
node* append(node* head, char* buffer, int col, int totCol){
    node* ptr = head;
    while(ptr->next != NULL){
        ptr = ptr->next;
    }
    node* newNode2 = create(buffer,col,totCol);
    ptr->next = newNode2;
    return head;
}

//Prints linked list
//Used for both testing and writing the sorted CSV at the end
void printLL(node* head, int totCol){
    int i;
    node* ptr = head;
    while(ptr != NULL){
        for(i = 0;i < totCol;i++){ 
		if(strcmp(ptr->row[i], "a") != 0){
	    		printf("%s",ptr->row[i]);
		}
		if(i != totCol-1){
			printf(",");
		}
        }
        printf("\n");
        ptr = ptr->next;
    }
}

//Puts 'a' as a place holder in any spots where the data is missing
char* fixWhiteSpace(char* buffer,int bufferSize){
    int cBuffer = 0;
    int cFixedBuffer = 0;
    int i;
    char* fixedBuffer = (char*)malloc(sizeof(char) * (bufferSize));
    while(buffer[cBuffer] == ','){
        fixedBuffer[cFixedBuffer] = 'a';
        fixedBuffer[cFixedBuffer + 1] = ',';
        cBuffer++;
        cFixedBuffer = cFixedBuffer + 2;
    }
    for(i = 0;i < strlen(buffer);i++){
       if(buffer[cBuffer] == ',' &&  buffer[cBuffer + 1] == ','){
 
           fixedBuffer[cFixedBuffer] = ',';
           fixedBuffer[cFixedBuffer + 1] = 'a';
           cBuffer++;
           cFixedBuffer = cFixedBuffer + 2;
       }
       else{
           fixedBuffer[cFixedBuffer] = buffer[cBuffer];
           cBuffer++;
           cFixedBuffer++;
       }
    } 
    return fixedBuffer;
}



//Reads in the CSV file and constructs a linked list using the data
node* readSTDIN(char* pathName, char* col, char* fileName, char* sortedDirectory){
    //int bufferSize = 450;
    if(*FATAL_ERROR == 1){
        exit(-1);
    }
    int bufferSize = 5000;
    char* token;
    char* buffer = (char*) malloc(sizeof(char)* bufferSize);
    char* headerBuffer = (char*)malloc(sizeof(char) * 450);
    int f = 0;
    char c;
    char** arrHeaders = (char**)malloc(sizeof(char*) * 28);
    int s;
    for(s = 0;s < 28;s++){
        arrHeaders[s] = (char*)malloc(sizeof(char) * 150);
    }            
 
    int a = 0;
    int totCol = 0;//num of cols
    int r = 0;//r is used to figure out what column we are sorting by
    node* head;
    int fd = open(pathName,O_RDONLY);
    if(fd < 0){
        write(STDERR,"Error: Can not open file\n",26);
        exit(-1);
    }
    else{
    
    while(read(fd, &c, 1) > 0){
        if (f >= bufferSize){
            bufferSize = bufferSize + 100;
            buffer = (char*)realloc(buffer, bufferSize);
        }
        if(c != '\n'){
            buffer[f] = c;
            f++;    
        }
        if(c == '\n'){
            
            if(a == 0){//a is the line number, so a = 0 is the header line
                token = strtok(buffer,",");
                while(token != NULL){
                    strcpy(arrHeaders[totCol],token);
                    if(strcmp(token,col) == 0){
                        r = totCol;
                    }
                    totCol++;
                    token = strtok(NULL,",");
                }
                int b;
                for(b = 0;b < totCol;b++){
                    strcat(headerBuffer,arrHeaders[b]);
                    if(b != totCol - 1){
                        strcat(headerBuffer,",");
                    }
                }
                strcat(headerBuffer,"\n"); 
            }
            if(a == 1){//First movie, so create the linked list
                if(buffer[strlen(buffer) - 1] == ','){
                    buffer[strlen(buffer)] = 'a';
                }
                strcpy(buffer,fixWhiteSpace(buffer,bufferSize));
                head = create(buffer,r,totCol);
        
            }
            if(a > 1){//Every movie after the first so add onto the current linked list
                if(buffer[strlen(buffer) - 1] == ','){
                    buffer[strlen(buffer)] = 'a';
                }
                strcpy(buffer,fixWhiteSpace(buffer,bufferSize));
                head = append(head,buffer,r,totCol);
            }
            a++;
            free(buffer);
            bufferSize = 5000;
            buffer = (char*)malloc(sizeof(char)*bufferSize);
            f = 0;
        }
    }
    char* newFileName = (char*)malloc(sizeof(char) * 300);
    fileName[strlen(fileName) - 4] = '\0';
    strcpy(newFileName, sortedDirectory);
    if(newFileName[strlen(newFileName)-1] != '/')//append / if not in folder name
        strcat(newFileName, "/");
    strcat(newFileName,fileName);
    strcat(newFileName,"-sorted-");
    strcat(newFileName,col);
    strcat(newFileName,".csv");
    strcat(newFileName,"\0");
    int fd2;
    fd2 = open(newFileName, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    node* ptr = mergesort(head);
    write(fd2,headerBuffer,strlen(headerBuffer));
    while(ptr){
        char* buffer = (char*) malloc(sizeof(char) * 10000);
        int i;
        for(i = 0; i < totCol; i++){
	    if(strcmp(ptr->row[i], "a") != 0)
                strcat(buffer, ptr -> row[i]);
            if(i != totCol - 1){
                strcat(buffer, ",");
            }
        }
        strcat(buffer, "\n");
        write(fd2,buffer,strlen(buffer));
        ptr = ptr->next;
        free(buffer);
    }
    free(headerBuffer);
    free(ptr);
    free(newFileName);
    int w;
    for(w = 0; w < 28; w++){
        free(arrHeaders[w]);
    }
    free(arrHeaders);
    free(head);
    free(token);
    }
}

void fileHandler(char* pathName, char* col, struct dirent *dent, char* sortedDirectory){
    if(*FATAL_ERROR == 1){
        exit(-1);
    }
    /*DIR *dir;
    dir = opendir(pathName);
    struct dirent * dent;
    dent = readdir(dir);*/
    char* fileName = (char*)malloc(sizeof(char) * 150);
    strcpy(fileName,dent->d_name);
    char* checkCSV = (char*)malloc(sizeof(char) * 4);
    checkCSV = pathName + strlen(pathName) - 4;
 
    (*childCount)++;
    int PID = fork();
    if(PID == 0){
	char buffer[30];
        int n = sprintf(buffer, "%d(F:%s),", getpid(), dent->d_name);
	write(STDOUT,buffer , n);
        if(strcmp(checkCSV,".csv") == 0 && !strstr(dent->d_name,"-sorted-") ){
            readSTDIN(pathName,col,fileName, sortedDirectory);
	}
        else if(strcmp(checkCSV,".csv") != 0){
            write(STDERR,"Error: Tried passing in file that was not a CSV\n",48);
        }
        exit(0);
    } 
}

//scans directory for files with the ".csv" extension and forks to sort them
void readDirectory(char* dirPath, char* col, char* outputDirPath){
    if(*FATAL_ERROR == 1){
        exit(-1);
    }
    pid_t wpid;
    int status = 0;
    DIR* dir;
    dir = opendir(dirPath);
    if(dir == NULL){
        write(STDERR,"Error: Can not open directory\n",31);
        exit(-1);
    }
    int PID;
    int otherPID;
    struct dirent *dent;
    //iterate through directory contents
    while(dent = readdir(dir)){
	//create file path 
        char newPath[300];
        strcpy(newPath, dirPath);
	if(newPath[strlen(newPath) - 1] != '/')
		strcat(newPath, "/");
        strcat(newPath, dent->d_name);
	//recurse if file is a folder, and doesn't 
        if(opendir(newPath) != NULL && strcmp("..", dent->d_name) != 0 && strcmp(".", dent->d_name) != 0){
                (*childCount)++;
	        int PID = fork();
		if(PID == 0){
		    readDirectory(newPath, col, outputDirPath);
	            char buffer[30];
                    int n = sprintf(buffer, "%d(D:%s),", getpid(), newPath);
	            write(STDOUT,buffer , n);
                    exit(0);
                }
	}
        else if (strcmp(".",dent->d_name) != 0 && strcmp("..",dent->d_name) != 0){
            fileHandler(newPath,col, dent, outputDirPath);
        }	
    }
    while((wpid = wait(&status)) > 0);
}


//Main method mostly does error checking to ensure parameters are correct
int main(int argc, char** argv){
     if (argc != 3 && argc != 5 && argc != 7){
          write(STDERR,"Error: Not correct amount of parameters\n",40);
          return 0;
     }	
     char* column_Names[28] = {"color","director_name","num_critic_for_reviews","duration","director_facebook_likes","actor_3_facebook_likes","actor_2_name","actor_1_facebook_likes","gross","genres","actor_1_name","movie_title","num_voted_users","cast_total_facebook_likes","actor_3_name","facenumber_in_poster","plot_keywords","movie_imdb_link","num_user_for_reviews","language","country","content_rating","budget","title_year","actor_2_facebook_likes","imdb_score","aspect_ratio","movie_facebook_likes"};
     int h = 0;
     int x = 0;
     int typeNum; 
      char* colSortBy = (char*)malloc(sizeof(char) * 50);
      char* readPath = (char*)malloc(sizeof(char) * 300);
      char* writePath = (char*)malloc(sizeof(char) * 300);
      int* flagsArr = (int*)malloc(sizeof(int) * 3);
      
      flagsArr[0] = 0;//-c Needs to be 1
      flagsArr[1] = 0;//-d If 0 then directory is not specified and read from current if 1 then directory is specified
      flagsArr[2] = 0;//-o If 0 then directory is not specified and write to current and if 1 then directory is specified
      if((strcmp(argv[1],"-c") == 0) || (strcmp(argv[1],"-d") == 0) || (strcmp(argv[1],"-o") == 0)){
          if((strcmp(argv[1],"-c") == 0)){
              flagsArr[0] = flagsArr[0] + 1;
              for(h = 0; h < 28; h++){
                  if(strcmp(argv[2],column_Names[h]) == 0){
                      x = 1;
                     typeNum = h;
                  }
              }
              if (x == 0){
                  write(STDERR,"Error: Parameters are not correct\n",34);
                  return -1;
              }
              strcpy(colSortBy,argv[2]);
          } 
          else if((strcmp(argv[1],"-d") == 0)){
              flagsArr[1] = flagsArr[1] + 1;
              strcpy(readPath, argv[2]);
          }
          else if((strcmp(argv[1],"-o") == 0)){
              flagsArr[2] = flagsArr[2] + 1;
              strcpy(writePath, argv[2]);
          }
      }
      else{
          write(STDERR,"Error: Parameters are not correct\n",34);
          return -1;
      }
      if(argc > 3){
          if((strcmp(argv[3],"-c") == 0) || (strcmp(argv[3],"-d") == 0) || (strcmp(argv[3],"-o") == 0)){
              if((strcmp(argv[3],"-c") == 0)){
                  flagsArr[0] = flagsArr[0] + 1;
                  for(h = 0; h < 28; h++){
                      if(strcmp(argv[4],column_Names[h]) == 0){
                          x = 1;
                         typeNum = h;
                      }
                  }
                  if (x == 0){
                      write(STDERR,"Error: Parameters are not correct\n",34);
                      return -1;
                  }
                  strcpy(colSortBy,argv[4]);
              }
              else if((strcmp(argv[3],"-d") == 0)){
                  flagsArr[1] = flagsArr[1] + 1;
                  strcpy(readPath, argv[4]);
              }
              else if((strcmp(argv[3],"-o") == 0)){
                  flagsArr[2] = flagsArr[2] + 1;
                  strcpy(writePath, argv[4]);
              }
          }
          else{
              write(STDERR,"Error: Parameters are not correct\n",34);
              return -1;
          }
     }
     if(argc > 5){
          if((strcmp(argv[5],"-c") == 0) || (strcmp(argv[5],"-d") == 0) || (strcmp(argv[5],"-o") == 0)){
              if((strcmp(argv[5],"-c") == 0)){
                  flagsArr[0] = flagsArr[0] + 1;
                  for(h = 0; h < 28; h++){
                      if(strcmp(argv[6],column_Names[h]) == 0){
                          x = 1;
                         typeNum = h;
                      }
                  }
                  if (x == 0){
                      write(STDERR,"Error: Parameters are not correct\n",34);
                      return -1;
                  }
                  strcpy(colSortBy,argv[6]);
              }   
              else if((strcmp(argv[5],"-d") == 0)){
                  flagsArr[1] = flagsArr[1] + 1;
                  strcpy(readPath, argv[6]);
              }
              else if((strcmp(argv[5],"-o") == 0)){
                  flagsArr[2] = flagsArr[2] + 1;
                  strcpy(writePath, argv[6]);
              }
          }
          else{
              write(STDERR,"Error: Parameters are not correct\n",34);
              return -1;
          }
      }
      if(flagsArr[0] != 1){
          write(STDERR,"Error: Parameters are not correct\n",34);
          return -1;
      }
      if((flagsArr[1] > 1) || (flagsArr[2] > 1)){
          write(STDERR,"Error: Parameters are not correct\n",34);
          return -1;
      }
      
      //map an address to shared memory with read and write permission
      childCount = mmap(NULL, sizeof(*childCount), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS ,-1, 0);
      FATAL_ERROR = mmap(NULL, sizeof(*childCount), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS ,-1, 0);
      *childCount = 0;
      *FATAL_ERROR = 0;
      //print out metadata and scan files
      char* anotherBuffer = (char*)malloc(sizeof(char) * 100);
      strcpy(anotherBuffer,"Initial PID:");
      char* parentPID = (char*)malloc(sizeof(char) * 25);
      sprintf(parentPID,"%d",getpid());
      strcat(anotherBuffer,parentPID);
      strcat(anotherBuffer,"\n"); 
      //printf("Initial PID:%d\n", getpid());
      write(STDOUT,anotherBuffer,35);
      char temp[] = "PIDS of all child processes:";
      write(STDOUT, temp, sizeof(temp));
      setType(typeNum);//sets sorting type based on given column title
      if(flagsArr[1] == 0){
          strcpy(readPath,"./");
      }
      if(flagsArr[2] == 0){
          strcpy(writePath,readPath);
      }
      readDirectory(readPath,colSortBy,writePath);
      wait(NULL);
      printf("\nTotal number of processes: %d\n", *childCount);
      //unmap shared memory
      munmap(childCount, sizeof(childCount));
      return 1;
}
