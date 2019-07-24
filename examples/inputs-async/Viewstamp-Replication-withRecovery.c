#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1

enum NormalOperations {Prepare_ROUND, PrepareOk_ROUND};

enum ViewChange {DoViewChange_ROUND, StartView_ROUND};

enum Recovery {Recovery_ROUND, RecoveryResponse_ROUND};

enum Status {normal, view_change, recovering};

enum Labels {Prepare, PrepareOk, DoViewChange, StartView, Recovery,
                RecoveryResponse};

typedef struct {
    int view_nr, op_number, commited;

    /* This message includes client_id and client req nr */
    char *message;
} log_str;

/* size - actual length; current - max buffer length */
typedef struct {
    int size, current;

    log_str **array;
} arraylist;

typedef struct {
    char *message;
    int request_nr;
    /* Data for piggy-backing */
    int commiting;
    int op_number;
} msg_NormalOp;

typedef struct {
    int log_size;
    log_str **log;
} msg_ViewChange;

typedef struct {
    int size;
    log_str **array;
} msg_Recovery;

typedef struct {
    int view_nr, replica_id;
    enum Labels label;

    void *content;
} abstract_message;

typedef struct lista {
    int size;
    abstract_message *info;
    struct lista *next;
} listA;

void abort() {
    exit(1);
}

int get_primary(int view_nr, int n) {
    return (view_nr % n);
}

char *in() {
    char *m = malloc(sizeof(char));
    return m;
}

char *prepare_ping() {
    char *m = malloc(5 * sizeof(char));

    if (!m) {
        abort();
    }

    strcpy(m, "ping");

    return m;
}

void send(abstract_message *mess, int n) {

}

/* Return from algorithm function */
void out_internal() {

}

void out_external(char *mess) {

}

log_str *create_log_entry(int view_nr, int op_number, char *message) {
    log_str *entry = malloc(sizeof(log_str));
    if (!entry) {
        abort();
    }

    entry->view_nr = view_nr;
    entry->op_number = op_number;

    entry->message = malloc(strlen(message) * sizeof(char));
    if (!entry->message) {
        free(entry);
        abort();
    }

    strcpy(entry->message, message);

    return entry;
}

arraylist *init_log() {
    arraylist *log;
    log = malloc(sizeof(arraylist));
    if (!log) {
        abort();
    }

    log->size = 0;

    log->array = malloc(2 * sizeof(log_str));
    if (!log->array) {
        abort();
    }
    log->current = 2;
    return log;
}

void dispose_log(arraylist **log) {
    arraylist *aux_log = *log;

    int i;
    for (i = 0; i < aux_log->size; i++) {
        if (aux_log->array[i]) {
            if (aux_log->array[i]->message) {
                free(aux_log->array[i]->message);
            }

            free(aux_log->array[i]);
        }
    }

    free(log);
}

/* Adds an entry to a log, and resizes log if necessary */
void add_entry_log(log_str *entry, arraylist *log) {
    log_str **aux;
    if (log->size == log->current) {
        log->current *= 2;

        aux = realloc(log->array, log->current * sizeof(log_str*));
        if (!aux) {
            if (log) {
                dispose_log(&log);
            }
            abort();
        }
        log->array = aux;
    }

    log->array[log->size] = entry;
    log->size++;
}

int timeout() { return 0; }

void reset_timeout() {}

abstract_message *recv() {
    return NULL;
}

msg_NormalOp *check_maj(listA *mbox, int op_number) {
    listA *it = mbox;
    msg_NormalOp *res, *aux;
    int nr = 0;
    while (it) {
        aux = (msg_NormalOp*)(it->info->content);
        if (aux->request_nr == op_number) {
            nr++;
            res = aux;
        }
        it = it->next;
    }

    if (nr == mbox->size)
        return res;

    return NULL;
}

void update_commit(arraylist *log, int op_number) {
    int i;
    for (i = 0; i < log->size; i++)
        if (log->array[i]->op_number == op_number) {
            log->array[i]->commited = 1;
            break;
        }
}

int nr_commited(int size, log_str **array) {
    int nr = 0, i;
    for (i = 0; i < size; i++) {
        if (array[i]->commited == 1)
            nr++;
    }

    return nr;
}

void write_to_disk(int v) {

}

int read_from_disk() {

}

int main(int argc, char **argv)
{
    /* Check for replica_id, IP and total nr of nodes as command line args */
    if (argc < 4) {
        printf("Not enough arguments!\n");
        return 0;
    }

    enum NormalOperations round;
    enum ViewChange bround;
    enum Recovery cround;

    msg_NormalOp *msgA = NULL;
    msg_ViewChange *msgB = NULL;
    msg_Recovery *msgC = NULL;
    listA *mboxA = NULL;
    listA *mboxB = NULL;
    listA *mboxC = NULL;

    arraylist *log = init_log();

    char *client_req = NULL;

    /* view_nr - current view, op_number - most recent received request */
    int view_nr = -1, op_number = 0;

    /* pid it is actually replica id */
    int pid = atoi(argv[1]);

    /* Nr of nodes */
    int n = atoi(argv[3]);

    int to_all = n+1;

    /* Start ViewChange algorithm for primary election */
    enum Status status = view_change;
    while (true) {
        /* Used by primary to get the log with max size */
        log_str **mess_log;
        int Max = -1;

        bround = DoViewChange_ROUND;
        view_nr++;

        if (pid == get_primary(view_nr, n)) {
            mboxB = NULL;
            while (true) {
                abstract_message *abs_m = recv();

                if (abs_m != NULL && abs_m->view_nr == view_nr && abs_m->label == DoViewChange) {
                    listA* mboxB_new = malloc(sizeof(listA));
                    if (!mboxB_new) {
                        abort();
                    }

                    mboxB_new->info = abs_m;
                    if (mboxB) {
                        mboxB_new->size = mboxB->size + 1;
                    } else {
                        mboxB_new->size = 1;
                    }

                    mboxB_new->next = mboxB;
                    mboxB = mboxB_new;
                    msgB = (msg_ViewChange *)(abs_m->content);
                    /* Get log with most commited operations*/
                    if (Max < nr_commited(msgB->log_size, msgB->log)) {
                        Max = nr_commited(msgB->log_size, msgB->log);
                        mess_log = msgB->log;
                    }
                } else if (abs_m != NULL && abs_m->view_nr > view_nr && label_algo(abs_m->label)) {

                    while (true) {
                        status = recovering;
                        cround = Recovery_ROUND;
                        view_nr = read_from_disk() + 1;

                        abs_m = malloc(sizeof(abstract_message));
                        if (!abs_m) {
                            abort();
                        }
                        abs_m->view_nr = view_nr;
                        abs_m->replica_id = pid;
                        send(abs_m, to_all);

                        cround = RecoveryResponse_ROUND;
                        while (true) {
                            abs_m = recv();

                            if (abs_m != NULL && abs_m->view_nr > view_nr && abs_m->label == RecoveryResponse) {
                                log->array = ((msg_Recovery*)(abs_m->content))->array;
                                log->size = log->current = ((msg_Recovery*)(abs_m->content))->size;
                            }

                            if (timeout()) {
                                out_internal();
                            }
                        }

                        cround = Recovery_ROUND;
                    }

                }

                if (timeout()) {
                    break;
                }

                if (mboxB != NULL && mboxB->size >= n / 2) {
                    log->array = mess_log;
                    log->size = log->current = Max;
                    break;
                }
            }

            if (mboxB != NULL && mboxB->size >= n / 2) {

                bround = StartView_ROUND;

                abstract_message *abs_m = malloc(sizeof(abstract_message));
                if (!abs_m) {
                    abort();
                }

                msgB = malloc(sizeof(msg_ViewChange));
                if (!msgB)
                    abort();

                abs_m->view_nr = view_nr;
                abs_m->label = StartView;
                msgB->log_size = log->size;
                msgB->log = log->array;
                abs_m->content = (void*)msgB;
                write_to_disk(view_nr);
                send(abs_m, to_all);

                /* Start Normal Op algo */
                while (true) {
                    status = normal;
                    round = Prepare_ROUND;

                    abstract_message *abs_m = malloc(sizeof(abstract_message));
                    if (!abs_m) {
                        abort();
                    }

                    msgA = malloc(sizeof(msg_NormalOp));
                    if (!msgA) {
                        abort();
                    }

                    /* Check if a request has been received from client */
                    client_req = in();

                    if (!client_req) {
                        msgA->message = prepare_ping();
                    } else {
                        msgA->message = malloc(strlen(client_req) * sizeof(char));
                        if (!msgA->message) {
                            abort();
                        }

                        strcpy(msgA->message, client_req);
                    }

                    msgA->commiting = 0;
                    if (log->size > 0 && log->array[log->size - 1]->commited == 1) {
                        msgA->commiting = 1;
                        msgA->op_number = log->array[log->size - 1]->op_number;
                    }

                    abs_m->view_nr = view_nr;
                    msgA->request_nr = op_number;
                    abs_m->replica_id = -1;
                    abs_m->label = Prepare;
                    abs_m->content = (void*)msgA;

                    if (client_req) {

                        add_entry_log(create_log_entry(view_nr, op_number, client_req),
                                        log);
                        op_number++;
                    }


                    send(abs_m, to_all);

                    free(msgA->message);
                    msgA->message = NULL;

                    free(msgA);
                    msgA = NULL;

                    free(abs_m);
                    abs_m = NULL;

                    round = PrepareOk_ROUND;

                    while (true) {
                        abstract_message *abs_m = recv();

                        if (abs_m != NULL && abs_m->view_nr == view_nr && abs_m->label == PrepareOk) {
                            listA* mboxA_new = malloc(sizeof(listA));
                            if (!mboxA_new) {
                                abort();
                            }

                            mboxA_new->info = abs_m;
                            if (mboxA) {
                                mboxA_new->size = mboxA->size + 1;
                            } else {
                                mboxA_new->size = 1;
                            }

                            mboxA_new->next = mboxA;
                            mboxA = mboxA_new;
                        }

                        if (timeout()) {
                            out_internal();
                        }

                        /* Check if I have a majority */
                        if (mboxA != NULL && mboxA->size >= n / 2) {
                            break;
                        }
                    }

                    if (mboxA != NULL && mboxA->size >= n / 2 && check_maj(mboxA, op_number) != NULL) {
                        log->array[log->size - 1]->commited = 1;
                    }

                    round = Prepare_ROUND;
                }
            }

        } else {
            abstract_message *abs_m = malloc(sizeof(abstract_message));
            if (!abs_m) {
                abort();
            }

            msgB = malloc(sizeof(msg_ViewChange));
            if (!msgB) {
                abort();
            }

            abs_m->view_nr = view_nr;
            abs_m->replica_id = pid;
            abs_m->label = StartView;
            msgB->log_size = log->size;
            msgB->log = log->array;
            abs_m->content = msgB;
            write_to_disk(view_nr);
            send(abs_m, get_primary(view_nr, pid));

            free(msgB);
            msgB = NULL;
            free(abs_m);
            abs_m = NULL;

            bround = StartView_ROUND;
            while (true) {
                abs_m = recv();

                if (abs_m != NULL && abs_m->view_nr == view_nr && abs_m->label == StartView) {
                    listA* mboxB_new = malloc(sizeof(listA));
                    if (!mboxB_new) {
                        abort();
                    }

                    mboxB_new->info = abs_m;
                    if (mboxB) {
                        mboxB_new->size = mboxB->size + 1;
                    } else {
                        mboxB_new->size = 1;
                    }

                    mboxB_new->next = mboxB;
                    mboxB = mboxB_new;
                }

                if (timeout()) {
                    break;
                }

                if (mboxB != NULL && mboxB->size == 1 && mboxB->next == NULL) {
                    log->size = ((msg_ViewChange*)(mboxB->info->content))->log_size;
                    log->array = ((msg_ViewChange*)(mboxB->info->content))->log;
                    break;
                }
            }

            if (mboxB != NULL && mboxB->size == 1 && mboxB->next == NULL) {
                /* Launch Normal Op algo */
                while (true) {
                    status = normal;
                    round = Prepare_ROUND;

                    while (true) {
                        abs_m = recv();

                        if (abs_m != NULL && abs_m->view_nr == view_nr &&
                            abs_m->label == Prepare &&
                            strcmp(((msg_NormalOp*)(abs_m->content))->message, "ping") != 0) {
                            listA *new_mboxA = malloc(sizeof(listA));

                            if (new_mboxA == NULL) {
                                abort();
                            }

                            new_mboxA->next = NULL;
                            if (mboxA) {
                                new_mboxA->size = mboxA->size + 1;
                            } else {
                                new_mboxA->size = 1;
                            }
                            new_mboxA->info = abs_m;
                            new_mboxA->next = mboxA;
                            mboxA = new_mboxA;
                            msgA = (msg_NormalOp*)abs_m->content;
                            if (msgA->commiting == 1)
                                update_commit(log, msgA->op_number);
                        } else {
                            reset_timeout();

                            if (msgA->commiting == 1)
                                update_commit(log, msgA->op_number);

                            free(msgA->message);
                            free(msgA);

                            free(abs_m);
                        }

                        if (timeout()) {
                            out_internal();
                        }

                        if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                            /* Update my log */
                            add_entry_log(create_log_entry(mboxA->info->view_nr,
                                        ((msg_NormalOp*)(mboxA->info->content))->request_nr,
                                        ((msg_NormalOp*)(mboxA->info->content))->message), log);
                            break;
                        }
                    }

                    if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                        round = PrepareOk_ROUND;
                        abs_m = malloc(sizeof(abstract_message));
                        msgA = malloc(sizeof(msg_NormalOp));

                        abs_m->label = PrepareOk;
                        abs_m->view_nr = view_nr;
                        abs_m->replica_id = pid;
                        msgA->request_nr = ((msg_NormalOp*)(mboxA->info->content))->request_nr;
                        abs_m->content = msgA;
                        send(abs_m, get_primary(view_nr, n));
                    }

                    round = Prepare_ROUND;
                }
            }
        }

        bround = DoViewChange_ROUND;
    }

    if (client_req) {
        free(client_req);
    }

    if (log) {
        dispose_log(&log);
    }

    if (msgA) {
        if (msgA->message) {
            free(msgA->message);
        }
        free(msgA);
    }

    if (msgB) {
        free(msgB);
    }
    return 0;
}
