
typedef struct Msg {
    int op;
    int epoch;
    int lab;
    int i;
    int round;
    int sender;
} msg;
typedef struct Ltype {
    int op;
    int commit;
} ltype;
typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;

void out(ltype *v);

int in();

int timeout();

int reset_timeout();

int rand_bool();

msg *recv();

void dispose(msg *c);

ltype *create_ltype(int op, int b);

void list_dispose_double(struct List *l);

void list_dispose_no_data(struct List *l);

void dispose_list(list *c);

void send(msg *message, int pid);

int leader(int phase, int net_size);

int all_agree(struct List *l);



enum round_typ_A {
    PROPOSE, ACK, LEADER, BCAST
};
enum round_typ_B {
    FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND
};

void main(int argc, int pid, struct arraylist *log, int lastIndex, int cmt_number, int leader, int epoch) {
    int n = argc;
    int to_all = n + 1;
    enum round_typ_B round;
    int i = lastIndex;
    int lab = BCAST;
    struct List *mbox;
    struct List *mbox_new;
    msg *m = NULL;
    round = AUX_ROUND;
    ltype *lastEntry = list_get(log, lastIndex);
    if (lastEntry != NULL && lastEntry->commit == true) {
        i++;
        lastIndex++;
        ltype *newEntry;
        if (pid == leader)
            newEntry = create_ltype(in(), false);
        else
            {
            newEntry = create_ltype(-1, false);

        list_add(log, newEntry);
        }
    } else {
    }
    while (true) {
        round = FIRST_ROUND;
        if (pid == leader) {
            m = (msg *) malloc(sizeof(msg));
            if (m == 0) {
            abort();
            }
            m->i = i;
            m->round = round;
            m->epoch = epoch;
            m->lab = BCAST;
            m->sender = leader;
            ltype *entry = list_get(log, lastIndex);
            if (entry != NULL) {
            m->op = entry->op;
            }
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        mbox = NULL;
        while (true) {
            m = recv();
            if (m != NULL && m->i == i && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                mbox_new = (list *) malloc(sizeof(list));
                if (mbox_new == 0) {
                abort();
                }
                mbox_new->message = m;
                if (mbox != 0)
                    {
                    mbox_new->size = mbox->size + 1;
                    }
                else {
                mbox_new->size = 1;
                mbox_new->next = mbox;
                mbox = mbox_new;
                }
            } else {
            free(m);
            }
            if (mbox != NULL && mbox->size >= 1) {
                if (mbox->message != NULL && mbox->message->sender == leader) {
                    break;
                }
            }
            if (timeout()) {
                break;
            }
        }
        if (mbox != NULL && mbox->size >= 1) {
            if (mbox->message != NULL && mbox->message->sender == leader) {
                if (pid != leader) {
                    ltype *logi = list_get(log, i);
                    if (logi != 0) {
                        logi->op = mbox->message->op;
                        logi->commit = false;
                    }
                }
            } else {
            }
            round = SECOND_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if (m == 0) {
            abort();
            }
            m->i = i;
            m->round = round;
            m->epoch = epoch;
            m->lab = BCAST;
            m->sender = pid;
            send(m, leader);
            dispose(m);
            m = NULL;
            list_dispose_no_data(mbox);
            mbox = NULL;
            if (pid == leader) {
                while (true) {
                    m = recv();
                    if (m != NULL && m->i == i && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                        mbox_new = (list *) malloc(sizeof(list));
                        if (mbox_new == 0) {
                        abort();
                        }
                        mbox_new->message = m;
                        if (mbox != 0)
                            {
                            mbox_new->size = mbox->size + 1;
                            }
                        else {
                        mbox_new->size = 1;

                        mbox_new->next = mbox;
                        mbox = mbox_new;
                        }
                    } else free(m);
                    if (timeout())
                        break;
                    if (mbox != NULL && mbox->size > n / 2) {
                        break;
                    }
                }
                if (mbox != NULL && mbox->size > n / 2) {
                    ltype *logi = list_get(log, i);
                    if (logi != 0) {
                    logi->commit = true;
                    }
                    cmt_number++;
                    int size = list_length(log);
                    out(logi);
                } else {
                    list_dispose_no_data(mbox);
                    mbox = NULL;
                    break;
                }
                round = THIRD_ROUND;
            } else {
                round = THIRD_ROUND;
            }
            if (pid == leader) {
                m = (msg *) malloc(sizeof(msg));
                if (m == 0) {
                abort();
                }
                m->i = i;
                m->round = round;
                m->epoch = epoch;
                m->lab = BCAST;
                m->sender = pid;
                send(m, to_all);
                dispose(m);
                m = NULL;
            }
            if (mbox != NULL) {
            list_dispose_no_data(mbox);

            mbox = NULL;
            }
            while (true) {
                m = recv();
                if (m != NULL && m->i == i && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                    mbox_new = (list *) malloc(sizeof(list));
                    if (mbox_new == 0) {
                    abort();
                    }
                    mbox_new->message = m;
                    if (mbox != 0)
                        {
                        mbox_new->size = mbox->size + 1;
                        }
                    else {
                    mbox_new->size = 1;

                    mbox_new->next = mbox;
                    mbox = mbox_new;
                    }
                } else free(m);
                if (mbox != NULL && mbox->size >= 1) {
                    if (mbox->message != NULL && mbox->message->sender == leader) {
                        break;
                    }
                }
                if (timeout()) {
                    break;
                }
            }
            if (mbox != NULL && mbox->size >= 1) {
                if (pid != leader) {
                    ltype *logi = list_get(log, i);
                    if (logi != 0)
                        {
                        logi->commit = true;
                        }
                    cmt_number++;
                    out(logi);
                }
                if (pid == leader) {
                    lastIndex++;
                    ltype *newEntry = create_ltype(in(), false);
                    list_add(log, newEntry);
                } else {
                    lastIndex++;
                    ltype *newEntry = create_ltype(-1, false);
                    list_add(log, newEntry);
                }
                list_dispose_no_data(mbox);
                mbox = NULL;
                i++;
                round = AUX_ROUND;
            } else {
                list_dispose_no_data(mbox);
                mbox = NULL;
                break;
            }
        } else {
            list_dispose_no_data(mbox);
            mbox = NULL;
            break;
        }
    }
}
