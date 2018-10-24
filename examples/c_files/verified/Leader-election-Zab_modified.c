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


int main(int argc, char **argv) {

    int n = argc;
    int to_all = n + 1;


    struct arraylist *log;
    log = create_arraylist();
    int lastIndex = list_length(log);


    enum round_typ_A round;
    int epoch;

    int pid;

    epoch = 0;
    round = CEpoch_ROUND;


    list *mbox = NULL;
    list *mbox_new = NULL;
    msg *m = NULL;


    while (true) {


        if (pid == leader(epoch, n)) {


            round = CEpoch_ROUND;

            reset_timeout();
            mbox = NULL;


            while (true) {

                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == CEpoch_ROUND) {
                    mbox_new = (list *) malloc(sizeof(list));
                    if (mbox_new == 0) {
                    abort();
                    }
                    mbox_new->message = m;
                    if (mbox != 0)
                        mbox_new->size = mbox->size + 1;
                    else mbox_new->size = 1;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                } else free(m);
                if (timeout()) {
                    break;
                }

                if (mbox != NULL && mbox->size > n / 2) {

                    break;
                }


            }


            if (mbox != NULL && mbox->size > n / 2) {
                epoch = max_epoch(mbox);


                epoch++;

                round = NewEpoch_ROUND;


            } else {
                list_dispose_mbox(mbox);
                mbox = NULL;
                epoch++;
                round = CEpoch_ROUND;
                break;
                round = AUX_ROUND;
            }


            m = (msg *) malloc(sizeof(msg));
            if (m == 0) {
            abort();
            }
            m->epoch = epoch;
            m->round = NewEpoch_ROUND;

            send(m, to_all);

            free(m);
            m = NULL;

            round = Ack_E_ROUND;

            list_dispose_mbox(mbox);
            mbox = NULL;

            while (true) {

                m = recv();
                if (m != NULL && m->epoch == epoch && m->round == Ack_E_ROUND) {
                    mbox_new = (list *) malloc(sizeof(list));
                    if (mbox_new == 0) {
                    abort();
                    }
                    mbox_new->message = m;
                    if (mbox != 0)
                        mbox_new->size = mbox->size + 1;
                    else mbox_new->size = 1;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                } else free(m);
                if (timeout()) {
                    break;
                }

                if (mbox != NULL && mbox->size > n / 2) {

                    break;
                }


            }


            if (mbox != NULL && mbox->size > n / 2) {

                lastIndex = max_log_size(mbox);
                struct arraylist *old_log = log;
                log = longest_log(mbox, lastIndex);
                list_dispose(old_log);


            } else {
                list_dispose_mbox(mbox);
                mbox = NULL;
                epoch++;
                round = CEpoch_ROUND;
                break;
            }


            epoch++;
            round = CEpoch_ROUND;


            list_dispose_mbox(mbox);
            mbox = NULL;


        } else {


            round = CEpoch_ROUND;

            m = (msg *) malloc(sizeof(msg));
            if (m == 0) {
            abort();
            }
            m->epoch = epoch;
            m->round = CEpoch_ROUND;

            send(m, leader(epoch, n));

            free(m);
            m = NULL;

            round = NewEpoch_ROUND;


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
                    if (mbox != 0)
                        mbox_new->size = mbox->size + 1;
                    else mbox_new->size = 1;
                    mbox_new->next = mbox;
                    mbox = mbox_new;

                } else free(m);
                if (timeout()) break;
                if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                    break;
                }

            }


            if (mbox != NULL && mbox->size == 1 && mbox->next == NULL) {
                epoch = mbox->message->epoch;


                round = Ack_E_ROUND;


            } else {
                list_dispose_mbox(mbox);
                mbox = NULL;
                epoch++;
                round = CEpoch_ROUND;
                break;

            }


            list_dispose_mbox(mbox);
            mbox = NULL;

            m = (msg *) malloc(sizeof(msg));
            if (m == 0) {
            abort();
            }
            m->epoch = epoch;
            m->round = Ack_E_ROUND;
            m->history = log;
            m->history_lenght = lastIndex;

            send(m, leader(epoch, n));

            free(m);
            m = NULL;


            epoch++;
            round = CEpoch_ROUND;


        }

    }
    list_dispose(log);

    return 1;
}

    

