typedef struct _msg {
	int round;
	int pid;
	int phase;
	int estimate;
	int timestamp;
	int ack;
	int sender;
} msg;


int main(int pid, int num, int estimate) {
	int state = 0;
	int phase = 0;
	int timestamp = 0;
	int estimate = in();
	int leader = 0;
	int phase = 0;
	int myid = pid;


	msg *mbox = null;
	int num_mbox = 0;
	int num_mbox_commit = 0;

	int retry;
	int timeout;

	volatile int random;

	while (state != 1) {
		leader = (phase % num) + 1;

		round = FIRST_ROUND;

		m = (msg *) malloc(sizeof(msg));
		m->phase = phase;
		m->round = round;
		m->estimate = estimate;
		m->sender = myid;
		m->timestamp = timestamp;

		send(m, to_leader);

		if (pid == leader) {
			num_mbox = 0;
			num_mbox_commit = 0;
			if (mbox) {
				free(mbox);
			}
			msg *mbox = (msg *) malloc(num * sizeof(msg));

			while (true) {
				m = recv();

				if (m->round == 1 && m->phase == phase) {
					mbox[num_mbox] = *m;
					num_mbox = num_mbox + 1;
					if (num_mbox >= (num + 1) / 2) {
						break;
					}
				}
				if (m->round == 4) {
					mbox[num_mbox] = *m;
					break;
				}
				if (timeout()) {
					break;
				}


			}


			if (mbox[num_mbox].round == 4) {
				round = FOURTH_ROUND;
				estimate = mbox[num_mbox].estimate;
				state = 1;
				break;
				round = AUX_ROUND;
			} else {
				m = max;
				m = max(m.ts, mbox);
				estimate = m.estimate;

			}

		}

		round = SECOND_ROUND;

		if (pid == leader) {
			msg *m = (msg *) malloc(sizeof(msg));
			m->sender = myid;
			m->phase = phase;
			m->round = round;
			m->estimate = estimate;
			send(m,to_all);
		}

		num_mbox = 0;
		if (mbox) {
			free(mbox);
		}
		msg *mbox = (msg *) malloc(num * sizeof(msg));


		while (1) {
			m = recv();
			if (m->round == 2 && m->phase == phase) {
				mbox[num_mbox] = *m;
				num_mbox = num_mbox + 1;
				if (m->sender == leader) {
					break;
				}
			}

			if (m->round == 4) {
				mbox[num_mbox] = *m;
				break;
			}
			if (timeout()) {
				break;
			}
		}

		if (mbox[num_mbox].round == 4) {
			round = FOURTH_ROUND;
			estimate = mbox[num_mbox].estimate;
			state = 1;
			break;
			round = AUX_ROUND;
		}
		if (mbox[num_mbox].round == 2) {
			estimate = mbox[num_mbox].estimate;
			timestamp = phase;
		}


		round = THIRD_ROUND;
		if (timestamp == phase) {
			msg *m = (msg *) malloc(sizeof(msg));
			m->sender = myid;
			m->phase = phase;
			m->round = round;
			send(m,to_leader);
		}


		if (pid == leader) {

			num_mbox = 0;
			while (1) {
				m = recv();
				if (m->round == 3 && m->phase == phase) {
					mbox_est[num_mbox] = *m;
					num_mbox = num_mbox + 1;
					if (num_mbox >= (num + 1) / 2) {
						break;
					}
				}


				if (m->round == 4) {
					mbox_est[num_mbox] = *m;
					break;
				}
				if (timeout()) {
					break;
				}
			}


			if (mbox_est[num_mbox].round == 4) {
				round = FOURTH_ROUND;
				estimate = mbox_est[num_mbox].estimate;
				state = 1;
				break;
				round = AUX_ROUND;
			}
			if (num_mbox >= (num + 1) / 2) {
				ack = 1;
			}
		}

		round = FOURTH_ROUND;

		if (pid == leader) {
			m = (msg *) malloc(sizeof(msg));
			m->sender = myidl;
			m->phase = phase;
			m->round = round;
			m->estimate = estimate;
			send(m,to_all);
		}

		while (1) {
			m = recv();
			if (m->round == 4) {

				break;
			}

			timeout = random;
			if (timeout()) {
				break;
			}
		}

		if (!timeout()) {
			estimate = m.estimate;
			state = 1;
			break;
		}

		phase = phase + 1;
		round = AUX_ROUND;
	}

}

