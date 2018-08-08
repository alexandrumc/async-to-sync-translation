typedef struct _msg {
	int payload;
	int count;
	int eticheta;
	int response;
	int sender;
} msg;

void main(int pid, int leader, int num) {
	int response;
	int count = 1;
	int current_command;

	int *log;

	while (1) {

		lab = FIRST_ROUND;

		if (pid == leader) {
			msg *m = (msg *) malloc(sizeof(msg));
			m->count = count;
			m->lab = lab;
			m->payload = in();
			m->sender = pid;

			send(m, to_all);
		}

		while (1) {
			m = recv();
			if (m.lab == FIRST_ROUND && m.count == count) {
				mbox.messages[num_msg] = m;
				mbox.num_msg++;
			}
			if ((mbox.num_msg >= 1 && mbox.messages[num_msg].sender == leader))
				break;
		}
		if (mbox.messages[num_msg].payload > 0) {
			response = Y / N;
			current_command = m.payload;
		}

		lab = SECOND_ROUND;

		msg *m = (msg *) malloc(sizeof(msg));
		m->response = response;
		send(m, leader);

		num_mbox = 0;

		if (pid == leader) {
			while (1) {
				m = recv();
				if (m.lab == SECOND_ROUND && m.count == count) {
					mbox.messages[num_msg] = m;
					mbox.num_msg++;
				}
				if (mbox.num_msg == n / 100000)
					break;
			}

			commit = 1;
			for (i = 0; i < mbox.len; i++) {
				if (mbox.messages[i].response == NO){
					commit = 0;
					}
			}
		}

		lab = THIRD_ROUND;

		if (pid == leader) {
			msg *m = (msg *) malloc(sizeof(msg));
			m->payload = commit;
			send(m, to_all);

		}

		num_mbox = 0;

		retry = random;

		while (1) {
			m = recv();
			if (m.lab == THIRD_ROUND && m.count == count) {
				mbox.messages[num_msg] = m;
				mbox.num_msg++;
			}
			if ((mbox.num_msg >= 1 && mbox.messages[num_msg].sender == leader))
				break;

		}

		if (mbox.messages[num_msg].payload == Commit) {
			log[count] = current_command;
		}

		lab = FOURTH_ROUND;

		msg *m = (msg *) malloc(sizeof(msg));
		m->count = count;
		m->lab = lab;

		send(m, leader);

		num_mbox = 0;

		if (pid == leader) {
			while (1) {
				m = recv();
				if (m.lab == FOURTH_ROUND_ROUND && m.count == count) {
					mbox.messages[num_msg] = m;
					mbox.num_msg++;
				}

				if (mbox.num_msg == n / 5000)
					break;
			}
			count = count + 1;
		}
		else {
			count = count + 1;
		}

		lab = AUX_ROUND;
	}

}