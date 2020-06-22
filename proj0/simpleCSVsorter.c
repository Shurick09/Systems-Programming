#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "simpleCSVsorter.h"
#include <errno.h>

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
    char* token = (char*)malloc(sizeof(char) * 150);
    char* sortBy = (char*)malloc(sizeof(char) * 150);
    token = strtok(buffer,",");//Breaks up the buffer into tokens to populate the linked list
    int i = 0; 
    int spec = 0;
    char* tempToken;
    while(i != totCol){
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
node* readSTDIN(char* col){
    int bufferSize = 450;
    char* buffer = (char*) malloc(sizeof(char)* bufferSize);
    int f = 0;
    char c;
    int a = 0;
    int totCol = 0;//num of cols
    int r = 0;//r is used to figure out what column we are sorting by
    node* head;
    while(read(STDIN, &c, 1) > 0){
        
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
                char* token;
                token = strtok(buffer,",");
                char** arrHeaders = (char**)malloc(sizeof(char*) * 28);
                int s;
                for(s = 0;s < 28;s++){
                    arrHeaders[s] = (char*)malloc(sizeof(char) * 150);
                }
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
                    printf("%s",arrHeaders[b]);
                    if(b != totCol - 1){
                        printf(",");
                    }
                }
                printf("\n"); 
               
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
            bufferSize = 450;
            buffer = (char*)malloc(sizeof(char)*bufferSize);
            f = 0;
        }
    }

    node* ptr = mergesort(head);
    printLL(ptr,totCol);//Prints final sorted linked list
    return head;
}


//Main method mostly does error checking to ensure parameters are correct
int main(int argc, char** argv){
     if (argc != 3){
          write(STDERR,"Error: Not correct amount of parameters\n",40);
          return 0;
     }
     if (strcmp(argv[1],"-c") != 0){
          write(STDERR,"Error: Parameters are not correct\n",34);
          return 0;
     }
     char* column_Names[28] = {"color","director_name","num_critic_for_reviews","duration","director_facebook_likes","actor_3_facebook_likes","actor_2_name","actor_1_facebook_likes","gross","genres","actor_1_name","movie_title","num_voted_users","cast_total_facebook_likes","actor_3_name","facenumber_in_poster","plot_keywords","movie_imdb_link","num_user_for_reviews","language","country","content_rating","budget","title_year","actor_2_facebook_likes","imdb_score","aspect_ratio","movie_facebook_likes"};
     int h = 0;
     int x = 0;
     int typeNum; 
     for(h = 0; h < 27; h++){
         if(strcmp(argv[2],column_Names[h]) == 0){
                x = 1;
		typeNum = h;
          }          
      }
      if (x == 0){          
          write(STDERR,"Error: Parameters are not correct\n",34);
          return 0;
      }
      setType(typeNum);
      readSTDIN(argv[2]);
      return 1;
      
}

