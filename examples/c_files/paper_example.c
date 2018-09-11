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
    epoch = 1;
    round = FIRST_ROUND;
    while(true){
        round = FIRST_ROUND;
        if (pid == leader)
        {
            m = (msg *) malloc(sizeof(msg));
            m->epoch = epoch;
            m->round = FIRST_ROUND;
            send(m, to_all);
        }
        reset_timeout(); 
        mbox = 0;
        while(true){
            tuplu_mesaj_pid = recv();
            if (m != NULL && m.epoch >= epoch && m.round == ELECT_ME)
                mbox = mbox + tuplu_mesaj_pid;
            if (timeout() || mbox != 0)
                break;
        }
        if (mbox != 0) {
            epoch = m.epoch;
            round = SECOND_ROUND;
            m = (msg *) malloc(sizeof(msg));
            m.epoch = epoch;
            m.round = SECOND_ROUND;
            send(m, to_all);
            reset_timeout();
            mbox = 0;
            while (true){
                tuplu_mesaj_pid = recv(cond_LEADER_IS);
                if (m != NULL) 
                    mbox = mbox + tuplu_mesaj_pid;
                if (timeout() || mbox >= n/2)
                    break;
            }
            if (mbox > n/2){
                log[epoch] = leader();
                out(epoch, leader());
            }
            epoch++;
            round = AUX_ROUND;
        }
        else {
            epoch ++;
            round = AUX_ROUND;
        }
    }
}
