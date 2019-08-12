#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum ViewChange {StartViewChange_ROUND, DoViewChange_ROUND, StartView_ROUND};

enum Labels {StartViewChange, DoViewChange, StartView};

typedef struct {
    int view_nr, op_number, commited;

    /* This message includes client_id and client req nr */
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

/* size - actual length; current - max buffer length */
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

/* Return from algorithm function */
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

void list_disposeB(listB **mbox)
{
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

void free_msg_ViewChange(msg_ViewChange *msg)
{
    int i = msg->log_size;
    for (i = 0; i < msg->log_size; i++)
        if (msg->log[i]) {
            if (msg->log[i]->message) {
                free(msg->log[i]->message);
            }
            free(msg->log[i]);
        }
    free(msg->log);
}

int nr_commited(int size, log_str **array)
{
    int nr = 0, i;
    for (i = 0; i < size; i++) {
        if (array[i]->commited == 1)
            nr++;
    }

    return nr;
}

log_str **choose_log(listB *mbox, int *Max)
{
    listB *it = mbox;
    log_str **mess_log;
    *Max = -1;
    while (it) {
        if (*Max < nr_commited(it->info->log_size, it->info->log)) {
            *Max = nr_commited(it->info->log_size, it->info->log);
            mess_log = it->info->log;
        }

        it = it->next;
    }

    return mess_log;
}

listB *last_elem(listB *mbox)
{
    listB *aux = NULL, *it = mbox;
    while (it) {
        aux = it;
        it = it->next;
    }

    return aux;
}

int count_of_view(listB *mbox, int view)
{
    listB *it = mbox;
    int nr = 0;
    while (it) {
        if (it->info->view_nr == view) {
            nr++;
        }
        it = it->next;
    }

    return nr;
}

listB *delete_msg_view(listB *mbox, int view)
{
    listB *it = mbox, *aux, *aux2;

    /* Delete from beginning */
    while (it && it->info->view_nr == view) {
        free_msg_ViewChange(it->info);
        aux = it->next;
        free(it);
        it = aux;
    }

    mbox = it;
    aux = NULL;

    while (it) {
        if (it->info->view_nr == view) {
            aux->next = it->next;
            free_msg_ViewChange(it->info);
            free(it);
            it = aux->next;
        } else {
            aux = it;
            it = it->next;
        }
    }
    return mbox;
}

int main(int argc, char **argv) {
    /* Check for replica_id, IP and total nr of nodes as command line args */
    if (argc < 4) {
        printf("Not enough arguments!\n");
        return 0;
    }

    enum ViewChange round;

    msg_ViewChange *msg = NULL;
    listB *mbox = NULL;

    arraylist *log = init_log();

    /* view_nr - current view, op_number - most recent received request */
    int view_nr = -1, prev_view;

    /* pid it is actually replica id */
    int pid = atoi(argv[1]);

    /* Nr of nodes */
    int n = atoi(argv[3]);

    int to_all = n+1;
    srand(time(NULL));
    view_nr = rand() % n;

    /* Start ViewChange algorithm for primary election */
    prev_view = view_nr;
    while (true) {

        round = StartViewChange_ROUND;
        list_disposeB(&mbox);
        mbox = NULL;

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
            }
            else if (msg != NULL && msg->view_nr == view_nr && msg->label == DoViewChange) {
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
                break;
            }
            else if (msg != NULL && msg->view_nr > view_nr &&
                (msg->label == StartViewChange || msg->label == DoViewChange)) {
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
                    break;
            }

            if (mbox != NULL && mbox->size > n / 2 && mbox->next == NULL) {
                break;
            }

            if (timeout()) {
                break;
            }
        }

        /* If I receive a StartViewChange message from a greater view I will
           jump to that view and StartViewChange round, considering that I
           need to help the quorum (so I ignore that I might be going to
           receive a DoViewChange as well). If I will get a timeout, at least
           I will be in the same view with the others for the next phase.
         */
        if (mbox != NULL && (last_elem(mbox))->info->view_nr > view_nr &&
            (last_elem(mbox))->info->label == StartViewChange) {
                prev_view = view_nr;
                view_nr = (last_elem(mbox))->info->view_nr;
                continue;
        }

        if ((mbox != NULL && mbox->size >= n / 2) ||
            (mbox != NULL && (last_elem(mbox))->info->view_nr >= view_nr &&
            (last_elem(mbox))->info->label == DoViewChange_ROUND)) {

                /* Delete msg for current view */
                delete_msg_view(mbox, view_nr);
                if (mbox != NULL && (last_elem(mbox))->info->view_nr > view_nr && (last_elem(mbox))->info->label == DoViewChange_ROUND) {
                        prev_view = view_nr;
                        view_nr = (last_elem(mbox))->info->view_nr;
                }

                round = DoViewChange_ROUND;

                if (pid == get_primary(view_nr, n)) {
                    /* Send to himself is irrelevant */
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
                        } else if (msg != NULL && msg->view_nr > view_nr && msg->label == DoViewChange) {
                            delete_msg_view(mbox, view_nr);
                            view = msg->view_nr;
                            reset_timeout();
                            continue;
                        } else if (msg != NULL && msg->view_nr > view_nr && msg->label == StartViewChange) {
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
                            break;
                        }

                        if (timeout()) {
                            break;
                        }

                        if (mbox != NULL && mbox->size > n / 2) {
                            break;
                        }
                    }

                    if (mbox != NULL && (last_elem(mbox))->info->view_nr > view_nr &&
                        (last_elem(mbox))->info->label == StartViewChange) {
                            prev_view = view_nr;
                            view_nr = (last_elem(mbox))->info->view_nr;
                            continue;
                    }

                    /* By considering jumping to StartViewChange, won't generate
                       infinte code. But jumping to DoViewChange would, because
                       I would have to duplicate the same receiving as above
                       and this would deliver the infinte code
                    */
                    if (mbox != NULL && mbox->size > n / 2) {
                        /* Choose log with max nr of commited ops */
                        int Max;
                        log->array = choose_log(mbox, &Max);
                        log->size = log->current = Max;
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

                        /* Start NormalOp */
                    }
                } else {
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
                                break;
                            }

                        if (timeout()) {
                            break;
                        }

                        if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                            break;
                        }
                    }

                    if (mbox != NULL && (last_elem(mbox))->info->view_nr > view_nr &&
                        (last_elem(mbox))->info->label == StartViewChange) {
                            prev_view = view_nr;
                            view_nr = msg->view_nr;
                            continue;
                        }

                    if (mbox != NULL && (last_elem(mbox))->info->view_nr > view_nr &&
                        (last_elem(mbox))->info->label == DoViewChange) {

                            delete_msg_view(mbox, view_nr);
                            prev_view = view_nr;
                            view_nr = (last_elem(mbox))->info->view_nr;

                            round = DoViewChange_ROUND;

                            /* Send to himself is irrelevant */
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
                                } else if (msg != NULL && msg->view_nr > view_nr && msg->label == DoViewChange) {
                                    delete_msg_view(mbox, view_nr);
                                    view = msg->view_nr;
                                    reset_timeout();
                                    continue;
                                } else if (msg != NULL && msg->view_nr > view_nr && msg->label == StartViewChange) {
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
                                    break;
                                }

                                if (timeout()) {
                                    break;
                                }

                                if (mbox != NULL && mbox->size > n / 2) {
                                    break;
                                }
                            }

                            if (mbox != NULL && (last_elem(mbox))->info->view_nr > view_nr &&
                                (last_elem(mbox))->info->label == StartViewChange) {
                                    prev_view = view_nr;
                                    view_nr = (last_elem(mbox))->info->view_nr;
                                    continue;
                            }

                            /* By considering jumping to StartViewChange, won't generate
                               infinte code. But jumping to DoViewChange would, because
                               I would have to duplicate the same receiving as above
                               and this would deliver the infinte code
                            */
                            if (mbox != NULL && mbox->size > n / 2) {
                                /* Choose log with max nr of commited ops */
                                int Max;
                                log->array = choose_log(mbox, &Max);
                                log->size = log->current = Max;
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

                                /* Start NormalOp */
                            }
                    } else {
                        if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                            log->size = mbox->info->log_size;
                            log->array = mbox->info->log;
                            list_disposeB(&mbox);
                            mbox = NULL;
                            /* Start Normal Op algo */
                        }
                    }
                }
        }
        round = StartViewChange_ROUND;
        view_nr++;
    }
    return 0;
}
