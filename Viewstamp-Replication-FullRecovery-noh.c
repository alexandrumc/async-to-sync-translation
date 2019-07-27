enum NormalOperations {Prepare_ROUND, PrepareOk_ROUND};
enum ViewChange {StartViewChange_ROUND, DoViewChange_ROUND, StartView_ROUND};
enum Recovery {Recovery_ROUND, RecoveryResponse_ROUND};
enum Status {normal, view_change, recovering};
enum Labels {Prepare, PrepareOk, StartViewChange, DoViewChange, StartView, Recovery,
                RecoveryResponse};
typedef struct {
    int view_nr, op_number, commited;
    char *message;
} log_str;
typedef struct {
    char *message;
    int request_nr, view_nr;
    int replica_id;
    enum Labels label;
    int commiting;
    int op_number;
} msg_NormalOp;
typedef struct {
    int view_nr, replica_id, log_size, prev_view, op_number;
    enum Labels label;
    log_str **log;
} msg_ViewChange;
typedef struct {
    int op_number, commited;
    char *message;
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
    int size, current;
    log_str **array;
} arraylist;
typedef struct comm {
    int replica_id, op_number, view_nr;
    struct comm *next;
} commit_list;
void abort() {
    exit(1);
}
const int true = 1;
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
int timeout2() { return 0; }
void reset_timeout() {}
void *recv() {
    return NULL;
}
msg_NormalOp *check_maj(listA *mbox, int op_number) {
    listA *it = mbox;
    msg_NormalOp *res;
    int nr = 0;
    while (it) {
        if (it->info->request_nr == op_number) {
            nr++;
            res = it->info;
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
void list_disposeA(listA **mbox) {
    listA *aux = *mbox, *n;
    while (aux) {
        if (aux->info && aux->info->message) {
            free(aux->info->message);
        }
        if (aux->info) {
            free(aux->info);
        }
        n = aux->next;
        free(aux);
        aux = n;
    }
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
int main(int argc, char **argv)
{
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
    listB *mboxB = NULL;
    commit_list *recovery_buffer = NULL;
    arraylist *log = init_log();
    char *client_req = NULL;
    int view_nr = -1, op_number = 0, prev_view;
    int transfer_to_algo = -1;
    int pid = atoi(argv[1]);
    int n = atoi(argv[3]);
    int to_all = n+1;
    enum Status status = view_change;
    prev_view = view_nr;
    while (true) {
        log_str **mess_log;
        int Max = -1;
        bround = StartViewChange_ROUND;
        view_nr++;
        transfer_to_algo = -1;
        list_disposeB(&mboxB);
        mboxB = NULL;
        if (pid == get_primary(view_nr, n)) {
            msgB = malloc(sizeof(msg_ViewChange*));
            if (!msgB) {
                abort();
            }
            msgB->view_nr = view_nr;
            msgB->replica_id = pid;
            msgB->label = StartViewChange;
            send((void*)msgB, to_all);
            while (true) {
                msgB = (msg_ViewChange*) recv();
                if (msgB != NULL && msgB->view_nr == view_nr && msgB->label == StartViewChange) {
                    listB *mboxB_new = malloc(sizeof(listB));
                    if (!mboxB_new) {
                        abort();
                    }
                    mboxB_new->info = msgB;
                    if (mboxB) {
                        mboxB_new->size = mboxB->size + 1;
                    } else {
                        mboxB_new->size = 1;
                    }
                    mboxB_new->next = mboxB;
                    mboxB = mboxB_new;
                } else if (msgB != NULL && msgB->view_nr > view_nr &&
                    msgB->label == PrepareOk) {
                        view_nr = msgB->view_nr;
                        log->array = msgB->log;
                        log->size = msgB->log_size;
                        log->current = log->size;
                        op_number = msgB->op_number;
                        transfer_to_algo = 1;
                    }
                if (timeout()) {
                    break;
                }
                if (mboxB != NULL && mboxB->size >= n / 2 && mboxB->next == NULL) {
                    break;
                }
            }
            if (mboxB != NULL && mboxB->size >= n / 2 && mboxB->next == NULL) {
                transfer_to_algo = -1;
                list_disposeB(&mboxB);
                mboxB = NULL;
                bround = DoViewChange_ROUND;
                mboxB = NULL;
                while (true) {
                    msgB = (msg_ViewChange*)recv();
                    if (msgB != NULL && msgB->view_nr == view_nr && msgB->label == DoViewChange) {
                        listB* mboxB_new = malloc(sizeof(listB));
                        if (!mboxB_new) {
                            abort();
                        }
                        mboxB_new->info = msgB;
                        if (mboxB) {
                            mboxB_new->size = mboxB->size + 1;
                        } else {
                            mboxB_new->size = 1;
                        }
                        mboxB_new->next = mboxB;
                        mboxB = mboxB_new;
                        if (Max < nr_commited(msgB->log_size, msgB->log)) {
                            Max = nr_commited(msgB->log_size, msgB->log);
                            mess_log = msgB->log;
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
                    list_disposeB(&mboxB);
                    mboxB = NULL;
                    bround = StartView_ROUND;
                    msgB = malloc(sizeof(msg_ViewChange));
                    if (!msgB) {
                        abort();
                    }
                    msgB->view_nr = view_nr;
                    msgB->label = StartView;
                    msgB->log_size = log->size;
                    msgB->log = log->array;
                    send((void*)msgB, to_all);
                    status = normal;
                    op_number = 0;
                    while (true) {
                        round = Prepare_ROUND;
                        recovery_buffer = NULL;
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
                        msgA->commiting = 0;
                        if (log->size > 0 && log->array[log->size - 1]->commited == 1) {
                            msgA->commiting = 1;
                            msgA->op_number = log->array[log->size - 1]->op_number;
                        }
                        msgA->view_nr = view_nr;
                        msgA->request_nr = op_number;
                        msgA->replica_id = -1;
                        msgA->label = Prepare;
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
                        while (true) {
                            msgA = (msg_NormalOp*) recv();
                            if (msgA != NULL && msgA->view_nr == view_nr && msgA->label == PrepareOk
                                && msgA->request_nr == op_number) {
                                listA* mboxA_new = malloc(sizeof(listA));
                                if (!mboxA_new) {
                                    abort();
                                }
                                mboxA_new->info = msgA;
                                if (mboxA) {
                                    mboxA_new->size = mboxA->size + 1;
                                } else {
                                    mboxA_new->size = 1;
                                }
                                mboxA_new->next = mboxA;
                                mboxA = mboxA_new;
                                commit_list *it = recovery_buffer;
                                while (it) {
                                    if (it->replica_id == msgA->replica_id) {
                                        it->view_nr = msgA->view_nr;
                                        it->op_number = msgA->request_nr;
                                        break;
                                    }
                                    it = it->next;
                                }
                                if (!it) {
                                    it = malloc(sizeof(commit_list));
                                    if (!it) {
                                        abort();
                                    }
                                    it->replica_id = msgA->replica_id;
                                    it->view_nr = msgA->view_nr;
                                    it->op_number = msgA->request_nr;
                                    it->next = recovery_buffer;
                                    recovery_buffer = it;
                                }
                            } else if (msgA != NULL && msgA->view_nr == view_nr && msgA->label == PrepareOk
                                && msgA->request_nr < op_number) {
                                commit_list *it = recovery_buffer;
                                while (it) {
                                    if (it->replica_id == msgA->replica_id) {
                                        break;
                                    }
                                    it = it->next;
                                }
                                if (it && it->op_number <= msgA->request_nr) {
                                    int index, i;
                                    if (!it) {
                                        index = 0;
                                    } else {
                                        index = it->op_number + 1;
                                    }
                                    i = index;
                                    while (true) {
                                        if (i == log->size) {
                                            break;
                                        }
                                        msgC = malloc(sizeof(msg_Recovery));
                                        if (!msgC) {
                                            abort();
                                        }
                                        msgC->op_number = log->array[index]->op_number;
                                        msgC->commited = log->array[index]->commited;
                                        strcpy(msgC->message, log->array[index]->message);
                                        send((void*) msgC, msgA->replica_id);
                                        i++;
                                    }
                                }
                            } else if (msgA != NULL && msgA->view_nr > view_nr &&
                                (msgA->label == DoViewChange || msgA->label == StartView)) {
                                    prev_view = view_nr;
                                    view_nr = msgA->view_nr - 1;
                                    list_disposeA(&mboxA);
                                    mboxA = NULL;
                                    out_internal();
                            } else if (msgA != NULL && msgA->view_nr < view_nr &&
                                msgA->label == StartViewChange) {
                                    msgB = malloc(sizeof(msg_ViewChange));
                                    if (!msgB) {
                                        abort();
                                    }
                                    msgB->view_nr = view_nr;
                                    msgB->label = PrepareOk;
                                    msgB->log_size = log->size;
                                    msgB->log = log->array;
                                    msgB->op_number = op_number;
                                    send((void*)msgB, msgA->replica_id);
                            }
                            if (timeout()) {
                                prev_view = view_nr;
                                list_disposeA(&mboxA);
                                mboxA = NULL;
                                out_internal();
                            }
                            if (mboxA != NULL && mboxA->size >= n / 2) {
                                break;
                            }
                        }
                        if (mboxA != NULL && mboxA->size >= n / 2) {
                            list_disposeA(&mboxA);
                            mboxA = NULL;
                            log->array[log->size - 1]->commited = 1;
                        }
                        round = Prepare_ROUND;
                    }
                }
            }
            if (transfer_to_algo == 1) {
                list_disposeB(&mboxB);
                mboxB = NULL;
                status = normal;
                op_number = 0;
                while (true) {
                    round = Prepare_ROUND;
                    while (true) {
                        msgA = (msg_NormalOp*)recv();
                        if (msgA != NULL && msgA->view_nr == view_nr &&
                            msgA->label == Prepare && msgA->request_nr == op_number &&
                            strcmp(msgA->message, "ping") != 0) {
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
                            if (msgA->commiting == 1) {
                                update_commit(log, msgA->op_number);
                            }
                        } else if (msgA != NULL && msgA->view_nr == view_nr &&
                                msgA->label == Prepare && msgA->request_nr == op_number &&
                                strcmp(msgA->message, "ping") == 0) {
                            reset_timeout();
                            if (msgA->commiting == 1)
                                update_commit(log, msgA->op_number);
                            free(msgA->message);
                            free(msgA);
                        } else if (msgA != NULL && msgA->view_nr == view_nr &&
                                msgA->label == Prepare && msgA->request_nr > op_number) {
                            while (true) {
                                msgC = (msg_Recovery *)recv();
                                if (msgC) {
                                    add_entry_log(create_log_entry(msgA->view_nr,
                                            msgC->op_number,
                                            msgC->message), log);
                                    log->array[log->size - 1]->commited = msgC->commited;
                                    op_number = msgC->op_number;
                                }
                                if (timeout2()) {
                                    break;
                                }
                            }
                        } else if (msgA != NULL && msgA->view_nr > view_nr &&
                            (msgA->label == DoViewChange || msgA->label == StartView)) {
                                prev_view = view_nr;
                                view_nr = msgA->view_nr - 1;
                                list_disposeA(&mboxA);
                                mboxA = NULL;
                                out_internal();
                        }
                        if (timeout()) {
                            prev_view = view_nr;
                            list_disposeA(&mboxA);
                            mboxA = NULL;
                            out_internal();
                        }
                        if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                            add_entry_log(create_log_entry(mboxA->info->view_nr,
                                        mboxA->info->request_nr,
                                        mboxA->info->message), log);
                            list_disposeA(&mboxA);
                            mboxA = NULL;
                            op_number++;
                            break;
                        }
                    }
                    if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                        list_disposeA(&mboxA);
                        mboxA = NULL;
                        round = PrepareOk_ROUND;
                        msgA = malloc(sizeof(msg_NormalOp));
                        msgA->label = PrepareOk;
                        msgA->view_nr = view_nr;
                        msgA->replica_id = pid;
                        msgA->request_nr = mboxA->info->request_nr;
                        send(msgA, get_primary(view_nr, n));
                    }
                    round = Prepare_ROUND;
                }
            }
            bround = StartViewChange_ROUND;
        }
        else {
            msgB = malloc(sizeof(msg_ViewChange*));
            if (!msgB) {
                abort();
            }
            msgB->view_nr = view_nr;
            msgB->replica_id = pid;
            msgB->label = StartViewChange;
            send((void*)msgB, to_all);
            while (true) {
                msgB = (msg_ViewChange*) recv();
                if (msgB != NULL && msgB->view_nr == view_nr && msgB->label == StartViewChange) {
                    listB* mboxB_new = malloc(sizeof(listB));
                    if (!mboxB_new) {
                        abort();
                    }
                    mboxB_new->info = msgB;
                    if (mboxB) {
                        mboxB_new->size = mboxB->size + 1;
                    } else {
                        mboxB_new->size = 1;
                    }
                    mboxB_new->next = mboxB;
                    mboxB = mboxB_new;
                } else if (msgB != NULL && msgB->view_nr > view_nr &&
                    (msgB->label == Prepare || msgB->label == PrepareOk)) {
                        view_nr = msgB->view_nr;
                        log->array = msgB->log;
                        log->size = msgB->log_size;
                        log->current = log->size;
                        op_number = msgB->op_number;
                        transfer_to_algo = 1;
                    }
                if (timeout()) {
                    break;
                }
                if (mboxB != NULL && mboxB->size >= n / 2 && mboxB->next == NULL) {
                    break;
                }
            }
            if (mboxB != NULL && mboxB->size >= n / 2 && mboxB->next == NULL) {
                transfer_to_algo = -1;
                list_disposeB(&mboxB);
                mboxB = NULL;
                bround = DoViewChange_ROUND;
                msgB = malloc(sizeof(msg_ViewChange));
                if (!msgB) {
                    abort();
                }
                msgB->view_nr = view_nr;
                msgB->replica_id = pid;
                msgB->label = DoViewChange;
                msgB->log_size = log->size;
                msgB->log = log->array;
                msgB->prev_view = prev_view;
                send(msgB, get_primary(view_nr, pid));
                bround = StartView_ROUND;
                while (true) {
                    msgB = (msg_ViewChange*) recv();
                    if (msgB != NULL && msgB->view_nr == view_nr && msgB->label == StartView) {
                        listB* mboxB_new = malloc(sizeof(listB));
                        if (!mboxB_new) {
                            abort();
                        }
                        mboxB_new->info = msgB;
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
                        log->size = mboxB->info->log_size;
                        log->array = mboxB->info->log;
                        break;
                    }
                }
                if (mboxB != NULL && mboxB->size == 1 && mboxB->next == NULL) {
                    list_disposeB(&mboxB);
                    mboxB = NULL;
                    status = normal;
                    op_number = 0;
                    while (true) {
                        round = Prepare_ROUND;
                        while (true) {
                            msgA = (msg_NormalOp*)recv();
                            if (msgA != NULL && msgA->view_nr == view_nr &&
                                msgA->label == Prepare && msgA->request_nr == op_number &&
                                strcmp(msgA->message, "ping") != 0) {
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
                                if (msgA->commiting == 1) {
                                    update_commit(log, msgA->op_number);
                                }
                            } else if (msgA != NULL && msgA->view_nr == view_nr &&
                                    msgA->label == Prepare && msgA->request_nr == op_number &&
                                    strcmp(msgA->message, "ping") == 0) {
                                reset_timeout();
                                if (msgA->commiting == 1)
                                    update_commit(log, msgA->op_number);
                                free(msgA->message);
                                free(msgA);
                            } else if (msgA != NULL && msgA->view_nr == view_nr &&
                                    msgA->label == Prepare && msgA->request_nr > op_number) {
                                while (true) {
                                    msgC = (msg_Recovery *)recv();
                                    if (msgC) {
                                        add_entry_log(create_log_entry(msgA->view_nr,
                                                msgC->op_number,
                                                msgC->message), log);
                                        log->array[log->size - 1]->commited = msgC->commited;
                                        op_number = msgC->op_number;
                                    }
                                    if (timeout2()) {
                                        list_disposeA(&mboxA);
                                        mboxA = NULL;
                                        break;
                                    }
                                }
                            } else if (msgA != NULL && msgA->view_nr > view_nr &&
                                (msgA->label == DoViewChange || msgA->label == StartView)) {
                                    prev_view = view_nr;
                                    view_nr = msgA->view_nr - 1;
                                    list_disposeA(&mboxA);
                                    mboxA = NULL;
                                    out_internal();
                            }
                            if (timeout()) {
                                prev_view = view_nr;
                                list_disposeA(&mboxA);
                                mboxA = NULL;
                                out_internal();
                            }
                            if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                                add_entry_log(create_log_entry(mboxA->info->view_nr,
                                            mboxA->info->request_nr,
                                            mboxA->info->message), log);
                                list_disposeA(&mboxA);
                                mboxA = NULL;
                                op_number++;
                                break;
                            }
                        }
                        if (mboxA != NULL && mboxA->size == 1 && mboxA->next == NULL) {
                            list_disposeA(&mboxA);
                            mboxA = NULL;
                            round = PrepareOk_ROUND;
                            msgA = malloc(sizeof(msg_NormalOp));
                            msgA->label = PrepareOk;
                            msgA->view_nr = view_nr;
                            msgA->replica_id = pid;
                            msgA->request_nr = mboxA->info->request_nr;
                            send(msgA, get_primary(view_nr, n));
                        }
                        round = Prepare_ROUND;
                    }
                }
            }
            bround = StartViewChange_ROUND;
        }
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
