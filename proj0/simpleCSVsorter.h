#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct node{
    char** row;
    char* sortBy;
    struct node* next;
}node;

//function prototype
node* mergesort(node* front);
void checkType(int n);
