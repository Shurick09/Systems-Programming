#include "multiThreadSorter_thread.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 0

//1 for sorting strings
//0 for sorting numerics
static int TypeOfSort = -1;

/*
void printLL(node* front){
	while(front){
		printf("%s", front->sortBy);
		if(front->next)
			printf("->");
		front = front->next;
	}
	printf("\n");
}
*/

//finds size of a linked list
int getListSize(node* front){
	int i = 0;
	node* ptr = front;
	while(ptr != NULL){
		i++;
		ptr = ptr->next;
	}
	return i;
}
//creates a linked list that begins at start and ends at end
void divideLL(node *front, node** lPart, node** rPart){
	int i;
	*lPart = front;
	node* ptr = front;
	int size = getListSize(front)-1;
	//go to midpoint
	for(i = 0; i < size/2; i++){
		ptr = ptr->next;
	}
	*rPart = ptr->next;
	ptr->next = NULL;
}
//sets type for sorting based on column name
void setType(int n){
	TypeOfSort = (n==0 || n==1|| n==6 || n==9 || n==10 || n==11 || n==14 
		|| n==16 || n==17 || n==19 || n==20 || n==21);

	if(DEBUG)
		printf("%d", TypeOfSort);
}

//returns 1 if str1 > str2
//returns 0 if otherwise
int compare(char* str1, char* str2){
	if(str1 == NULL || strcmp("a", str1) == 0){
		return 0;
	}
	else if(str2 == NULL || strcmp("a", str2) == 0){
		return 1;	
	}
	//inputs are strings
	if(TypeOfSort){
		
		//compare strings
		if(DEBUG)
			printf("sorting by strings(%s\t%s)\n", str1, str2);
		if(strcmp(str1, str2) > 0)
			return 1;
		else
			return 0;
	}
	//inputs are numerical
	else{
		float num1 = atof(str1);
		float num2 = atof(str2);
		if(DEBUG)
			printf("sorting by nums(%f\t%f)\n",num1,num2);
		return num1 > num2;
	}
}
//combines two ordered linked lists)
node* merge(node* lPart, node* rPart){
	//pointers to first and last elements of new list
	node* firstNew = (node*) malloc(sizeof(node));
	node* lastNew = firstNew;

	if(DEBUG && lPart == NULL && rPart == NULL)
		printf("Error: both partitions are empty\n");
		
	int isRunning = 1;
	while(isRunning){
		//if partition is empty add elements from other partition
		if(lPart == NULL){
			lastNew->next = rPart;
			isRunning = 0;
		}
		else if(rPart == NULL){
			lastNew->next = lPart;
			isRunning = 0;
		}
		else if(compare(lPart->sortBy, rPart->sortBy)){
			lastNew->next = rPart;
			rPart = rPart -> next;
			lastNew = lastNew->next;
		}
		else{
			lastNew->next = lPart;
			lPart = lPart->next;
			lastNew = lastNew->next;
		}
	}
	return firstNew -> next;
}
//recursively split and merge list
node* mergesort(node* front){	
	if(TypeOfSort < 0){
		printf("Warning: setType must be called first\n");
	}
	if(front == NULL || front->next == NULL)
		return front;
	if(DEBUG)
		printf("Front Length:%d\t", getListSize(front));
	//split list into lPart and rPart
	node* rPart;
	node* lPart;
	divideLL(front, &lPart, &rPart);
	//show list sizes
	if(DEBUG)
		printf("LeftLength:%d\tRightLength:%d\n", getListSize(lPart), getListSize(rPart));
	//sort and merge partitions
	return merge(mergesort(lPart),mergesort(rPart));
}
