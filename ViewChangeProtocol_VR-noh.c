enum ViewChange {StartViewChange_ROUND, DoViewChange_ROUND, StartView_ROUND};
enum Labels {StartViewChange, DoViewChange, StartView};
typedef struct {
    int view_nr, op_number, commited;
    char *message;
} log_str;
typedef struct {
    int view_nr, replica_id, log_size, prev_view, op_number;
    enum Labels label;
    log_str **log;
} msg_ViewChange;
typedef struct listb {
    int size;
    msg_ViewChange *info;
    struct listb *next;
} listB;
typedef struct {
    int size, current;
    log_str **array;
} arraylist;
void abort() {
    exit(1);
}
const int true = 1;
int get_primary(int view_nr, int n) {
    return (view_nr % n);
}
void send(void *mess, int n) {
}
void out_internal() {
}
void out_external(char *mess) {
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
int timeout() { return 0; }
int timeout2() { return 0; }
void reset_timeout() {}
void *recv() {
    return NULL;
}
void list_disposeB(listB **mbox) {
    listB *aux = *mbox, *n;
    int i;
    while (aux) {
        if (aux->info) {
            for (i = 0; i < aux->info->log_size; i++) {
                if (aux->info->log[i]) {
                    if (aux->info->log[i]->message) {
                        free(aux->info->log[i]->message);
                    }
                    free(aux->info->log[i]);
                }
            }
            free(aux->info->log);
            free(aux->info);
        }
        n = aux->next;
        free(aux);
        aux = n;
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
int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Not enough arguments!\n");
        return 0;
    }
    enum ViewChange round;
    msg_ViewChange *msg = NULL;
    listB *mbox = NULL;
    arraylist *log = init_log();
    int view_nr = -1, prev_view;
    int pid = atoi(argv[1]);
    int n = atoi(argv[3]);
    int to_all = n+1;
    srand(time(NULL));
    view_nr = rand() % n;
    prev_view = view_nr;
    while (true) {
        log_str **mess_log;
        int Max = -1;
        view_nr++;
        round = StartViewChange_ROUND;
        list_disposeB(&mbox);
        mbox = NULL;
        if (pid == get_primary(view_nr, n)) {
            msg = malloc(sizeof(msg_ViewChange*));
            if (!msg) {
                abort();
            }
            msg->view_nr = view_nr;
            msg->replica_id = pid;
            msg->label = StartViewChange;
            send((void*)msg, to_all);
            while (true) {
                msg = (msg_ViewChange*) recv();
                if (msg != NULL && msg->view_nr == view_nr && msg->label == StartViewChange) {
                    listB *mboxB_new = malloc(sizeof(listB));
                    if (!mboxB_new) {
                        abort();
                    }
                    mboxB_new->info = msg;
                    if (mbox) {
                        mboxB_new->size = mbox->size + 1;
                    } else {
                        mboxB_new->size = 1;
                    }
                    mboxB_new->next = mbox;
                    mbox = mboxB_new;
                } else if (msg != NULL && msg->view_nr > view_nr &&
                    (msg->label == StartViewChange || msg->label == DoViewChange)) {
                        break;
                    }
                if (timeout()) {
                    break;
                }
                if (mbox != NULL && mbox->size >= n / 2 && mbox->next == NULL) {
                    break;
                }
            }
            if (msg != NULL && msg->view_nr > view_nr &&
                (msg->label == StartViewChange || msg->label == DoViewChange)) {
                    view_nr = msg->view_nr;
                    round = StartViewChange_ROUND;
                }
            if (mbox != NULL && mbox->size >= n / 2 && mbox->next == NULL) {
                list_disposeB(&mbox);
                mbox = NULL;
                round = DoViewChange_ROUND;
                mbox = NULL;
                while (true) {
                    msg = (msg_ViewChange*)recv();
                    if (msg != NULL && msg->view_nr == view_nr && msg->label == DoViewChange) {
                        listB* mboxB_new = malloc(sizeof(listB));
                        if (!mboxB_new) {
                            abort();
                        }
                        mboxB_new->info = msg;
                        if (mbox) {
                            mboxB_new->size = mbox->size + 1;
                        } else {
                            mboxB_new->size = 1;
                        }
                        mboxB_new->next = mbox;
                        mbox = mboxB_new;
                        if (Max < nr_commited(msg->log_size, msg->log)) {
                            Max = nr_commited(msg->log_size, msg->log);
                            mess_log = msg->log;
                        }
                    }
                    if (timeout()) {
                        break;
                    }
                    if (mbox != NULL && mbox->size >= n / 2) {
                        log->array = mess_log;
                        log->size = log->current = Max;
                        break;
                    }
                }
                if (mbox != NULL && mbox->size >= n / 2) {
                    list_disposeB(&mbox);
                    mbox = NULL;
                    round = StartView_ROUND;
                    msg = malloc(sizeof(msg_ViewChange));
                    if (!msg) {
                        abort();
                    }
                    msg->view_nr = view_nr;
                    msg->label = StartView;
                    msg->log_size = log->size;
                    msg->log = log->array;
                    send((void*)msg, to_all);
                }
            }
            round = StartViewChange_ROUND;
        }
        else {
            msg = malloc(sizeof(msg_ViewChange*));
            if (!msg) {
                abort();
            }
            msg->view_nr = view_nr;
            msg->replica_id = pid;
            msg->label = StartViewChange;
            send((void*)msg, to_all);
            while (true) {
                msg = (msg_ViewChange*) recv();
                if (msg != NULL && msg->view_nr == view_nr && msg->label == StartViewChange) {
                    listB* mboxB_new = malloc(sizeof(listB));
                    if (!mboxB_new) {
                        abort();
                    }
                    mboxB_new->info = msg;
                    if (mbox) {
                        mboxB_new->size = mbox->size + 1;
                    } else {
                        mboxB_new->size = 1;
                    }
                    mboxB_new->next = mbox;
                    mbox = mboxB_new;
                } else if (msg != NULL && msg->view_nr > view_nr &&
                    (msg->label == StartViewChange || msg->label == DoViewChange)) {
                        break;
                    }
                if (timeout()) {
                    break;
                }
                if (mbox != NULL && mbox->size >= n / 2 && mbox->next == NULL) {
                    break;
                }
            }
            if (msg != NULL && msg->view_nr > view_nr &&
                (msg->label == StartViewChange || msg->label == DoViewChange)) {
                    view_nr = msg->view_nr;
                    round = StartViewChange_ROUND;
                }
            if (mbox != NULL && mbox->size >= n / 2 && mbox->next == NULL) {
                list_disposeB(&mbox);
                mbox = NULL;
                round = DoViewChange_ROUND;
                msg = malloc(sizeof(msg_ViewChange));
                if (!msg) {
                    abort();
                }
                msg->view_nr = view_nr;
                msg->replica_id = pid;
                msg->label = DoViewChange;
                msg->log_size = log->size;
                msg->log = log->array;
                msg->prev_view = prev_view;
                send(msg, get_primary(view_nr, pid));
                round = StartView_ROUND;
                while (true) {
                    msg = (msg_ViewChange*) recv();
                    if (msg != NULL && msg->view_nr == view_nr && msg->label == StartView) {
                        listB* mboxB_new = malloc(sizeof(listB));
                        if (!mboxB_new) {
                            abort();
                        }
                        mboxB_new->info = msg;
                        if (mbox) {
                            mboxB_new->size = mbox->size + 1;
                        } else {
                            mboxB_new->size = 1;
                        }
                        mboxB_new->next = mbox;
                        mbox = mboxB_new;
                    } else if (msg != NULL && msg->view_nr > view_nr &&
                        (msg->label == StartViewChange || msg->label == DoViewChange)) {
                            break;
                        }
                    if (timeout()) {
                        break;
                    }
                    if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                        log->size = mbox->info->log_size;
                        log->array = mbox->info->log;
                        break;
                    }
                }
                if (msg != NULL && msg->view_nr > view_nr &&
                    (msg->label == StartViewChange || msg->label == DoViewChange)) {
                        view_nr = msg->view_nr;
                        round = StartViewChange_ROUND;
                    }
                if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                    list_disposeB(&mbox);
                    mbox = NULL;
                }
            }
            round = StartViewChange_ROUND;
        }
    }
    return 0;
}
