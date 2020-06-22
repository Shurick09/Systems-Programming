#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef enum{
    FALSE,
    TRUE
} boolean;

typedef struct _account{
    char* accountName;
    double currBalance;
    boolean inSession;
    struct _account* next;
} account;

typedef struct LLofFD{
    int fd;
    struct LLofFD* next;
}LLofFD;
