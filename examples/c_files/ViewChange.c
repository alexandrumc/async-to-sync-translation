typedef struct _msg {
    int round;
    int pid;
    int epoch;
    int *history;
    int history_lenght;
    int commit_nb;
    int sender;
} msg;

typedef enum ROUNDS {
    START_VIEW_CHANGE_ROUND, DO_VIEW_CHANGE_ROUND, START_VIEW_ROUND, PREPARE_OK_ROUND
};

int main(int pid, int num_processes) {
    int round = START_VIEW_CHANGE_ROUND;
    int view = 0;
    int commit;
    int *history;
    int history_lenght;

    int leader;

    msg *mbox = malloc(2 * num_processes, sizeof(msg));
    int num_mbox = 0;
    int retry;

    while (1) {

        round = START_VIEW_CHANGE_ROUND;
        view++;
        leader = view % num_processes;

        if (rand() % 2 == 0) {
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = START_VIEW_CHANGE_ROUND;
            send(m, all);
            round = AUX_ROUND;
        }


        memset(mbox, 0, sizeof(mbox));
        num_mbox = 0;

        while (true) {
            m = recv();
            if (m != NULL && m.view == view && m.round == START_VIEW_CHANGE_ROUND) {
                mbox[num_mbox] = m;
                num_mbox++;
            }
            if (m != NULL && m.view > view) {
                break;
            }
            if (timeout() || (num_mbox >= num_processes / 2)) {
                break;
            }
        }


        if (num_mbox >= num_processes / 2) {
            round = DO_VIEW_CHANGE_ROUND;
        } else {
            if (m.view > view) {
                view = m.view;
                round = START_VIEW_CHANGE_ROUND;
                continue;
                round = AUX_ROUND;
            } else {
                round = AUX_ROUND;
            }
        }

        if (round == DO_VIEW_CHANGE_ROUND) {
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = DO_VIEW_CHANGE_ROUND;
            m->history = history;
            m->history_lenght = history_lenght;
            m->commit_nb = commit_nb;
            m->old_view = old_view;
            send(m, leader);
        }

        if (leader == pid) {


            memset(mbox, 0, sizeof(mbox));
            num_mbox = 0;

            while (true) {
                m = recv();
                if (m != NULL && m.view >= view && m.round == round) {
                    mbox[num_mbox] = m;
                    num_mbox++;
                }
                if (timeout() || num_mbox >= num_processes / 2) {
                    break;
                }
            }

            if (num_mbox >= num_processes / 2) {
                view = get_view(mbox);
                history = get_longest_history(mbox);
                commit_nb = get_biggest_commit(mbox);
                round = START_VIEW_ROUND;
            } else {
                round = START_VIEW_CHANGE_ROUND;
                continue;
                round = AUX_ROUND;
            }
        }


        if (leader == pid) {
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = round;
            m->history = history;
            m->history_lenght = history_lenght;
            m->commit_nb = commit_nb;
            send(m, to_all);
        }

        memset(mbox, 0, sizeof(mbox));
        num_mbox = 0;

        while (true) {
            m = recv();
            if (m != NULL && m.view >= view && m.round == round) {
                mbox[num_mbox] = m;
                num_mbox++;
            }
            if (timeout() || num_mbox >= 1) {
                break;
            }
        }

        if (num_mbox >= 1) {
            view = mbox[num_mbox].view;
            round = mbox[num_mbox].round;
            history = mbox[num_mbox].history;
            history_lenght = mbox[num_mbox].history_lenght;
            if (unprepared_log(history)) {
                round = PREPARE_OK_ROUND;
            }
        } else {
            round = START_VIEW_CHANGE_ROUND;
            continue;
            round = AUX_ROUND;
        }


        if (round == PREPARE_OK_ROUND) {
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = round;
            m->history = history;
            m->history_lenght = history_lenght;
            m->commit_nb = commit_nb;
            send(m, leader);
        }
        if (round == START_VIEW_ROUND || round == PREPARE_OK_ROUND) {
            NormalOp();
        }
        round = AUX_ROUND;
    }
}
