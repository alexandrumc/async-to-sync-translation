
typedef struct Msg {
    int round;
    int phase;
    int payload;
    int response;
    int sender;
    int commit;
} msg;


typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;


void out(int v1, int v2);


int in();


int rand_bool();


msg *recv();

void dispose(msg *c);


void list_dispose(struct List *l);


void list_dispose_no_data(struct List *l);

void dispose_list(list *c);

void send(msg *message, int pid);


int leader(int phase, int net_size);


int all_agree(struct List *l);