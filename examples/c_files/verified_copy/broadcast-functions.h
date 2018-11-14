


typedef struct Msg {
    int op;
    int epoch;
    int  lab;
    int i;
    int round;
    int sender;
} msg;

typedef struct Ltype {
    int op;
    int commit;
} ltype;


typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

void out(ltype *v);


int in();




int timeout();

int reset_timeout();

int rand_bool();



msg* recv();

void dispose(msg* c);


ltype * create_ltype(int op, int b);

void list_dispose_double(struct List *l);

void list_dispose_no_data(struct List *l);

void dispose_list(list* c);

void send(msg* message, int pid);

int leader(int phase, int net_size);

int all_agree(struct List* l);
