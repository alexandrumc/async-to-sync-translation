#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1

enum NormalOperations {Prepare_ROUND, PrepareOk_ROUND};

enum ViewChange {DoViewChange_ROUND, StartView_ROUND};

enum Recovery {Recovery_ROUND, RecoveryResponse_ROUND};

enum Status {normal, view_change, recovering};

typedef struct {
    /* Consider piggy backed in message */
    char *message;
    int request_nr, view_nr;
    int replica_id;
} msg_NormalOp;

typedef struct {
    int aux;
} msg_ViewChange;

typedef struct {
    int aux;
} msg_Recovery;

typedef struct lista {
    int size;
    msg_NormalOp *info;
    struct lista *next;
} listA;

typedef struct listb {
    int size;
    msg_ViewChange *info;
    struct listb *next;
} listB;

typedef struct listc {
    int size;
    msg_Recovery *info;
    struct listb *next;
} listC;

typedef struct {
    int view_nr, op_number, commited;

    /* This message includes client_id and client req nr */
    char *message;
} log_str;

typedef struct {
    int size, current;

    log_str **array;
} arraylist;

typedef struct comm {
    int op_number, count;
    comm *next;
} commit_list;

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

void send(void *mess, int n) {

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

void *recv() {
    return NULL;
}

int main(int argc, char **argv)
{
    /* Check for IP, replica_id and total nr of nodes as command line args */
    if (argc < 4) {
        printf("Not enough arguments!\n");
        return 0;
    }

    enum Status status = normal;

    enum NormalOperations round;
    enum ViewChange bround;
    enum Recovery cround;

    msg_NormalOp *msgA = NULL;
    listA *mboxA = NULL;

    arraylist *log = init_log(), *aux_log = init_log();

    char *client_req = NULL;

    /* view_nr - current view, op_number - most recent received request */
    int view_nr = 0, op_number = 0;

    /* pid it is actually replica id */
    int pid = atoi(argv[1]);

    /* Nr of nodes */
    int n = atoi(argv[3]);

    int to_all = n+1;

    /* Start normal operation */
    while (true) {
        round = Prepare_ROUND;
        if (pid == get_primary(view_nr, n)) {
            msgA = malloc(sizeof(msg_NormalOp));
            if (!msgA) {
                abort();
            }

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

            msgA->view_nr = view_nr;
            msgA->request_nr = op_number;
            msgA->replica_id = -1;

            if (client_req) {

                add_entry_log(create_log_entry(view_nr, op_number, client_req),
                                log);
                op_number++;
            }


            send((void*)msgA, to_all);

            free(msgA->message);
            msgA->message = NULL;

            free(msgA);
            msgA = NULL;

            round = PrepareOk_ROUND;
            commit_list l = NULL;

            //while (true) {

            //}
            round = Prepare_ROUND;
        } else {
            mboxA = NULL;
            reset_timeout();
            /* Wait for Prepare message from primary */
            while (true) {
                msgA = (msg_NormalOp*)recv();

                if (msgA != NULL && strcmp(msgA->message, "ping") != 0) {
                    listA *new_mboxA = malloc(sizeof(msg_NormalOp));

                    if (new_mboxA == NULL) {
                        abort();
                    }

                    new_mboxA->next = NULL;
                    if (mboxA) {
                        new_mboxA->size = mboxA->size + 1;
                    } else {
                        new_mboxA->size = 1;
                    }
                    new_mboxA->info = msgA;
                    new_mboxA->next = mboxA;
                    mboxA = new_mboxA;
                } else {
                    free(msgA->message);
                    free(msgA);
                }

                if (timeout()) {
                    break;
                }

                if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                    break;
                }
            }

            if (timeout()) {
                /* Launch ViewChange algo */
            }

            if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                round = PrepareOk_ROUND;
                if (mboxA->info->view_nr > view_nr) {
                    /* Launch recovery algo */
                }

                listA *mess_list = NULL, *mess_node = NULL;
                if (mboxA->info->view_nr == view_nr){
                    if (log->size + 1 == mboxA->info->request_nr) {
                        add_entry_log(create_log_entry(mboxA->info->view_nr,
                                    mboxA->info->request_nr,
                                    mboxA->info->message), log);

                        /* Create PrepareOk message for this entry */
                        mess_node = malloc(sizeof(listA));
                        if (!mess_node) {
                            abort();
                        }

                        mess_node->info->message = malloc(strlen(mboxA->info->message) * sizeof(char));
                        if (!mess_node->info->message) {
                            abort();
                        }

                        strcpy(mess_node->info->message, mboxA->info->message);
                        mess_node->info->view_nr = mboxA->info->view_nr;
                        mess_node->info->replica_id = pid;
                        mess_node->info->request_nr = mboxA->info->request_nr;

                        if (mess_list) {
                            mess_node->size = mess_list->size + 1;
                        } else {
                            mess_node->size = 1;
                        }

                        mess_node->next = mess_list;
                        mess_list = mess_node;

                        int sw = 1;
                        int i;
                        /* Added a new entry, check if we can put other entries
                            from the aux buffer */
                        while (sw == 1) {
                            i = 0;
                            while (i < aux_log->size) {
                                if (aux_log->array[i]->op_number == log->size + 1) {
                                    break;
                                }
                                i++;
                            }

                            if (i < aux_log->size) {
                                add_entry_log(create_log_entry(aux_log->array[i]->view_nr,
                                            aux_log->array[i]->op_number,
                                            aux_log->array[i]->message), log);

                                /* Create a PrepareOk message for this entry */
                                mess_node = malloc(sizeof(listA));
                                if (!mess_node) {
                                    abort();
                                }

                                mess_node->info->message = malloc(strlen(aux_log->array[i]->message) * sizeof(char));
                                if (!mess_node->info->message) {
                                    abort();
                                }

                                strcpy(mess_node->info->message, aux_log->array[i]->message);
                                mess_node->info->view_nr = aux_log->array[i]->view_nr;
                                mess_node->info->replica_id = pid;
                                mess_node->info->request_nr = aux_log->array[i]->op_number;

                                if (mess_list) {
                                    mess_node->size = mess_list->size + 1;
                                } else {
                                    mess_node->size = 1;
                                }

                                mess_node->next = mess_list;
                                mess_list = mess_node;
                            } else {
                                sw = 0;
                            }
                        }
                    } else {
                        add_entry_log(create_log_entry(mboxA->info->view_nr,
                                    mboxA->info->request_nr,
                                    mboxA->info->message), aux_log);
                    }

                }
                listA *it = mess_list;
                while (it) {
                    /* PrepOk message and send to primary */
                    msgA = malloc(sizeof(msg_NormalOp));
                    if (!msgA)
                        abort();

                    msgA->view_nr = it->info->view_nr;
                    msgA->request_nr = it->info->request_nr;
                    msgA->replica_id = pid;
                    strcpy(msgA->message, it->info->message);
                    send((void*)msgA, get_primary(view_nr, n));
                    it = it->next;
                }
            }
            round = Prepare_ROUND;
        }
    }

    if (client_req) {
        free(client_req);
    }

    if (msgA) {
        if (msgA->message) {
            free(msgA->message);
        }
        free(msgA);
    }

    if (log) {
        dispose_log(&log);
    }
    return 0;
}
