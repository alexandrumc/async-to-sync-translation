typedef struct _msg {
    int round;
    int pid;
    int epoch;
    int estimate;
    int timestamp;
    int ack;
    int sender;
} msg;

int main() {
    epoch = 0;
    while (true) {
        round = FIRST_ROUND;
        if (pid == leader) {
            m = (msg *) malloc(sizeof(msg));
            m->epoch = epoch;
            m->round = FIRST_ROUND;
            send(m, to_all);
        }
        reset_timeout();
        mbox = 0;
        while (true) {
            tuplu_mesaj_pid = recv();
            if (m != NULL && m.epoch >= epoch && m.round == ELECT_ME) {
                mbox = mbox + tuplu_mesaj_pid;
            }
            if (timeout() || mbox != 0) {
                break;
            }
        }
        if (mbox != 0) {
            epoch = m.epoch;
            round = SECOND_ROUND;
            x++;
            m = (msg *) malloc(sizeof(msg));
            m.epoch = epoch;
            m.round = SECOND_ROUND;
            send(m, to_all);
            reset_timeout();
            mbox = 0;
            while (true) {
                tuplu_mesaj_pid = recv(cond_LEADER_IS);
                if (m != NULL) {
                    mbox = mbox + tuplu_mesaj_pid;
                }
                if (timeout() || mbox >= n / 2) {
                    break;
                }
            }
            if (mbox > n / 2) {
                log[epoch] = leader();
                msg *mbox;
                int num_mbox = 0;


                while (true) {
                    round = FIRST_ROUND;

                    if (pid == leader) {

                        m = (msg *) malloc(sizeof(msg));
                        m->i = lastIndex;
                        m->round = round;
                        m->view = view;
                        m->lab = lab;
                        m->op = log[lastIndex][0];
                        m->sender = leader;

                        send(m, to_all);
                    }

                    while (true) {
                        m = recv();
                        if (m->i == lastIndex && m->view == view && m->round == round && m->lab == lab &&
                            m->sender == leader) {

                            mbox[num_mbox] = *m;
                            num_mbox = num_mbox + 1;
                        }

                        if (num_mbox == 1)
                            {
                            break;
                            }

                        if (timeout())
                            {
                            break;
                            }
                    }

                    if (timeout()) {
                        break;
                    }

                    if (num_mbox == 1) {

                        log[lastIndex][0] = mbox[num_mbox].op;
                        log[lastIndex][1] = false;

                        round = SECOND_ROUND;
                        m = (msg *) malloc(sizeof(msg));
                        m->i = lastIndex;
                        m->round = round;
                        m->view = view;
                        m->lab = lab;
                        m->sender = pid;
                        send(m, to_leader);

                        if (pid == leader) {
                            while (true) {
                                m = recv();

                                if (m->i == lastIndex && m->view == view && m->round == round && m->lab == lab) {
                                    mbox[num_mbox] = *m;
                                    num_mbox++;
                                }

                                if (timeout())
                                    {
                                    break;
                                    }

                                if (num_box > (n / 2))
                                    {
                                    break;
                                    }
                            }

                            if (num_box > (n / 2)) {
                                log[i][1] = true;
                                out_external(log[i][0]);
                            } else {
                                break;
                            }
                            round = THIRD_ROUND;
                        } else {
                            round = THIRD_ROUND;
                        }

                        if (pid == leader) {
                            m = (msg *) malloc(sizeof(msg));
                            m->i = i;
                            m->round = round;
                            m->view = view;
                            m->lab = lab;
                            m->sender = pid;
                            send(m, to_all);
                        }

                        while (true) {
                            m = recv();
                            if (m->i == lastIndex && m->view == view && m->round == round && m->lab == lab &&
                                m->sender == leader) {

                                mbox[num_mbox] = *m;
                                num_mbox = num_mbox + 1;
                            }

                            if (num_mbox == 1)
                                {
                                break;
                                }

                            if (timeout())
                                {
                                break;
                                }

                        }

                        if (timeout()) {
                            break;
                        }

                        if (num_mbox == 1) {
                            log[lastIndex][1] = true;
                            log[lastIndex + 1][1] = false;
                            log[lastIndex + 1][0] = in();
                            lastIndex++;
                            round = AUX_ROUND;
                        }
                    }
                }
            }
            epoch++;
            round = AUX_ROUND;
        } else {
            epoch++;
            round = AUX_ROUND;
        }
    }
}
