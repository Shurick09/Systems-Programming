#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct node{
    char** row;
    char* sortBy;
    struct node* next;
}node;

//function prototypes
node* mergesort(node* front);
node* merge(node* lPart, node* rPart);
void checkType(int n);
