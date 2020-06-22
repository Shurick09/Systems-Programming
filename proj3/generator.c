#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

inline void swap(char *x, char *y) {
	char t = *x; *x = *y; *y = t;
}

char* reverse(char *buffer, int i, int j)
{
	while (i < j)
		swap(&buffer[i++], &buffer[j--]);

	return buffer;
}

char* itoa(int value, char* buffer, int base)
{
    if (base < 2 || base > 32)
		return buffer;
int n = abs(value);

	int i = 0;
	while (n)
	{
		int r = n % base;

		if (r >= 10) 
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / base;
	}
if (i == 0)
		buffer[i++] = '0';


if (value < 0 && base == 10)
		buffer[i++] = '-';

	buffer[i] = '\0';


}
static char *rand_string(char *str, size_t size)
{
    size_t n;
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...123456789_?/,.[]()` ";
    if (size) {
        --size;
        for (n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}


void main(){
    char commands[6][20] = {"create","serve","deposit","withdraw","query","end"};
    time_t t;
    int nameCount = 0;
    int random;
    int random2;
    int random3;
    char finalList[100][255];
    char* name = (char*)malloc(sizeof(char) * 15);
    char buffer[300];
    char buffer2[300];
    char buffer3[300];
    char numBuffer[300];
    char nameList[10000][255];
    char* tempName = (char*)malloc(sizeof(char) * 15);
    srand((unsigned) time(&t));
    //time_t t2;
    //srand((unsigned) time(&t2));
    int i;
    for(i = 0;i < 99; i++){
        random = rand() % 6;
        if(strcmp(commands[random],"query") == 0 || strcmp(commands[random],"end") == 0 || strcmp(commands[random],"quit") == 0){
            strcpy(finalList[i],commands[random]);
        }
        else if(strcmp(commands[random],"create") == 0){
            name = rand_string(tempName,15);
            strcpy(nameList[nameCount],name);
            nameCount++;
            strcpy(buffer,commands[random]);
            strcat(buffer," ");
            strcat(buffer,name);
            strcpy(finalList[i],buffer);
        }
        else if(strcmp(commands[random],"deposit") == 0 || strcmp(commands[random],"withdraw") == 0){
            random3 = rand();
            itoa(random3,numBuffer,10);
            strcpy(buffer3,commands[random]);
            strcat(buffer3," ");
            strcat(buffer3,numBuffer);
            strcpy(finalList[i],buffer3);
        }
        else{
            if(nameCount != 0){
                random2 = rand() % nameCount;
                strcpy(buffer,commands[random]);
                strcat(buffer," ");
                strcat(buffer,nameList[random2]);
                strcpy(finalList[i],buffer);
            } 
        }
    }
    strcpy(finalList[99],"quit");
    int j;
    for(j = 0; j < 100; j++){
        printf("%s\n",finalList[j]);
    }
}
