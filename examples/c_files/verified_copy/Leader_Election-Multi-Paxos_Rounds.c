enum round_typ_A {
    NewEpoch, Ack_E, New_Leader, Ack_LD, BCAST
};
typedef struct Msg {
    int round;
    int pid;
    int epoch;
    struct arraylist *history;
    int history_lenght;
    int sender;
} msg;
typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;
enum round_typ_B {
    FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND
};
typedef struct MsgB {
    int op;
    int epoch;
    int lab;
    int i;
    int round;
    int sender;
} msgb;
typedef struct Ltype {
    int op;
    int commit;
} ltype;
typedef struct ListB {
    msgb *message;
    struct ListB *next;
    int size;
} listb;
struct arraylist;

struct arraylist *create_arraylist();

ltype *list_get(struct arraylist *a, int i);

int list_length(struct arraylist *a);

void list_add(struct arraylist *a, void *v);

int reset_timeout();

int coord();

int timeout();

msg *recv();

void send(msg *message, int pid);

int max_log_size(struct List *mbox);

struct arraylist *longest_log(struct List *mbox, int lastIndex);

void list_dispose_data(struct arraylist *a);

int main(int argc, char **argv) {
    int n = argc;
    int to_all = n + 1;
    struct arraylist *log;
    log = create_arraylist();
    int lastIndex = list_length(log);
    enum round_typ_A round;
    int epoch;
    int pid,coord;
    epoch = 0;
    round = NewEpoch;
    list *mbox = NULL;
    list *mbox_new = NULL;
    msg *m = NULL;
    
    while (true) {
        round = NewEpoch;
        if (pid == coord()) {
            m = (msg *) malloc(sizeof(msg));
            if (m == 0) {
                abort();
            }
            m->epoch = epoch;
            m->round = NewEpoch;
            m->sender = pid;
            send(m, to_all);
            free(m);
            m = NULL;
            coord = pid;
        } else {coord = -1;}
        mbox = NULL;
        reset_timeout();
        while (true) {
            m = recv();
            if (m != NULL && m->epoch >= epoch && m->round == NewEpoch) {
                mbox_new = (list *) malloc(sizeof(list));
                if (mbox_new == 0) {
                    abort();
                }
                mbox_new->message = m;
                if (mbox != 0) {
                    mbox_new->size = mbox->size + 1;
                } else {
                    mbox_new->size = 1;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }
            } else {
                free(m);
            }
            if (timeout()) {
                break;
            }
            if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                break;
            }
        }
        if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
            epoch = mbox->message->epoch;
            leader = mbox->message->sender;
            list_dispose_mbox(mbox);
            mbox = NULL;
            round = Ack_E;
        
   
            m = (msg *) malloc(sizeof(msg));
            if (m == 0) {
                abort();
            }
            m->epoch = epoch;
            m->round = Ack_E;
            m->history = log;
            m->history_lenght = lastIndex;
            send(m, leader);
            free(m);
            m = NULL;
            if (pid == coord) {
                while (true) {
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == Ack_E) {
                        mbox_new = (list *) malloc(sizeof(list));
                        if (mbox_new == 0) {
                            abort();
                        }
                        mbox_new->message = m;
                        if (mbox != 0) {
                            mbox_new->size = mbox->size + 1;
                        } else {
                            mbox_new->size = 1;
                            mbox_new->next = mbox;
                            mbox = mbox_new;
                        }
                    } else {
                        free(m);
                    }
                    if (timeout()) {
                        break;
                    }
                    if (mbox != NULL && mbox->size > n / 2) {
                        break;
                    }
                }
                if (mbox != NULL && mbox->size > n / 2) {
                    lastIndex = max_log_size(mbox);
                    log = longest_log(mbox, lastIndex);
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    round = New_Leader;
                } else {
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    epoch++;
                    round = AUX_ROUND;
                }
            } else {
                round = New_Leader;
            }
            if (round == New_Leader) {
                if (pid == coord) {
                    m = (msg *) malloc(sizeof(msg));
                    if (m == 0) {
                        abort();
                    }
                    m->epoch = epoch;
                    m->round = New_Leader;
                    m->history = log;
                    m->history_lenght = lastIndex;
                    m->sender = pid;
                    send(m, to_all);
                    free(m);
                    m = NULL;
                }
                mbox = NULL;
                reset_timeout();
                while (true) {
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == New_Leader) {
                        mbox_new = (list *) malloc(sizeof(list));
                        if (mbox_new == 0) {
                            abort();
                        }
                        mbox_new->message = m;
                        if (mbox != 0) {
                            mbox_new->size = mbox->size + 1;
                        } else {
                            mbox_new->size = 1;
                            mbox_new->next = mbox;
                            mbox = mbox_new;
                        }
                    } else {
                        free(m);
                    }
                    if (timeout()) {
                        break;
                    }
                    if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                        break;
                    }
                }
                if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                    lastIndex = mbox->message->history_lenght;
                    log = longest_log(mbox, lastIndex);
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    round = Ack_LD;
                } else {
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    epoch++;
                    round = NewEpoch;
                }
                if (round == Ack_LD) {
                    
                    
                    epoch++;
                    round = NewEpoch;
                }
            }
        } else {
            list_dispose_mbox(mbox);
            mbox = NULL;
            epoch++;
            round = NewEpoch;
        }
    }
    return 1;
}

void list_dispose_mbox(struct List *l);

void send_msgb(msgb *message, int pid);

msgb *recv_msgb();

void listB_dispose_no_data(struct ListB *l);

void out(ltype *v);

int in();

ltype *create_ltype(int op, int b);
