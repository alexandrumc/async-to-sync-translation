typedef struct _msg
{
  int round;
  int pid;
  int epoch;
  int *history;
  int history_lenght;
  int sender;
} msg;
int main(int pid, int num_processes)
{
  iter = 0;
  b2 = true;
  b1 = true;
  int lab = 0;
  int epoch = 0;
  int history;
  int history_lenght;
  int leader;
  msg_curr_e * mbox_curr_e[2 * num_processes];
  int num_mbox_curr_e = 0;
  msg_curr_e * m_curr_e;
  msg_new_e * mbox_new_e[2 * num_processes];
  int num_mbox_new_e = 0;
  msg_new_e * m_new_e;
  msg_ack_e * mbox_ack_e[2 * num_processes];
  int num_mbox_ack_e = 0;
  msg_ack_e * m_ack_e;
  msg_new_l * mbox_new_l[2 * num_processes];
  int num_mbox_new_l = 0;
  msg_new_l * m_new_l;
  msg_ack_l * mbox_ack_l[2 * num_processes];
  int num_mbox_ack_l = 0;
  msg_ack_l * m_ack_l;
  msg_com * mbox_cmt[2 * num_processes];
  int num_mbox_cmt = 0;
  msg_com * m_com;
  int retry;
  while (1)
  {
    iter++;
    if (iter == epoch)
    {
      if (!b1)
      {
        round = THIRD_ROUND;
        m = (msg *) malloc(sizeof(msg));
        m->epoch = epoch;
        m->round = round;
        m->history = history;
        m->history_lenght = history_lenght;
        send(m, leader);
        if (leader == pid)
        {
          memset(mbox_ack_e, 0, sizeof(mbox_ack_e));
          if (num_mbox_ack_e >= (num_processes / 2))
          {
            history = get_longest_history(num_mbox_ack_e);
            history_lenght = lenght(history);
          }
          else
          {
            round = FIRST_ROUND;
            continue;
          }

        }

        round = FORTH_ROUND;
        if (leader == pid)
        {
          m = (msg *) malloc(sizeof(msg));
          m->epoch = epoch;
          m->round = round;
          m->history = history;
          m->history_lenght = history_lenght;
          send(m, to_all);
        }

        memset(mbox_new_l, 0, sizeof(mbox_new_l));
        if (num_mbox_new_l >= 1)
        {
          history = mbox_new_e[num_mbox_new_l].history;
          history_lenght = mbox_new_e[num_mbox_new_l].history_lenght;
        }
        else
        {
          round = FIRST_ROUND;
          continue;
        }

        Synchronization();
        b1 = true;
      }
      else
      {
        if (!b2)
        {
          round = SECOND_ROUND;
          if (leader == pid)
          {
            m = (msg *) malloc(sizeof(msg));
            m->epoch = epoch;
            m->round = round;
            send(p, to_all);
          }

          memset(mbox_new_e, 0, sizeof(mbox_new_e));
          if (num_mbox_new_e >= 1)
          {
            epoch = mbox_new_e[num_mbox_new_e].epoch;
            if (iter != epoch)
            {
              b1 = false;
              continue;
            }
            else
            {
              b1 = true;
            }

          }
          else
          {
            round = FIRST_ROUND;
            continue;
          }

          round = THIRD_ROUND;
          m = (msg *) malloc(sizeof(msg));
          m->epoch = epoch;
          m->round = round;
          m->history = history;
          m->history_lenght = history_lenght;
          send(m, leader);
          if (leader == pid)
          {
            memset(mbox_ack_e, 0, sizeof(mbox_ack_e));
            if (num_mbox_ack_e >= (num_processes / 2))
            {
              history = get_longest_history(num_mbox_ack_e);
              history_lenght = lenght(history);
            }
            else
            {
              round = FIRST_ROUND;
              continue;
            }

          }

          round = FORTH_ROUND;
          if (leader == pid)
          {
            m = (msg *) malloc(sizeof(msg));
            m->epoch = epoch;
            m->round = round;
            m->history = history;
            m->history_lenght = history_lenght;
            send(m, to_all);
          }

          memset(mbox_new_l, 0, sizeof(mbox_new_l));
          if (num_mbox_new_l >= 1)
          {
            history = mbox_new_e[num_mbox_new_l].history;
            history_lenght = mbox_new_e[num_mbox_new_l].history_lenght;
          }
          else
          {
            round = FIRST_ROUND;
            continue;
          }

          Synchronization();
          b2 = true;
        }
        else
        {
          round = FIRST_ROUND;
          leader = rand() % num_processes;
          m = (msg *) malloc(sizeof(msg));
          m->epoch = epoch;
          m->round = FIRST_ROUND;
          send(m, leader);
          if (leader == pid)
          {
            reset_timeout();
            memset(mbox_curr_e, 0, sizeof(mbox_curr_e));
            if (num_mbox_curr_e >= (num_processes / 2))
            {
              new_epoch = get_max_epoch(num_mbox_curr_e) + 1;
              epoch = new_epoch;
              if (iter != epoch)
              {
                b2 = false;
                continue;
              }
              else
              {
                b2 = true;
              }

            }
            else
            {
              round = FIRST_ROUND;
              continue;
            }

          }

          round = SECOND_ROUND;
          if (leader == pid)
          {
            m = (msg *) malloc(sizeof(msg));
            m->epoch = epoch;
            m->round = round;
            send(p, to_all);
          }

          memset(mbox_new_e, 0, sizeof(mbox_new_e));
          if (num_mbox_new_e >= 1)
          {
            epoch = mbox_new_e[num_mbox_new_e].epoch;
            if (iter != epoch)
            {
              b1 = false;
              continue;
            }
            else
            {
              b1 = true;
            }

          }
          else
          {
            round = FIRST_ROUND;
            continue;
          }

          round = THIRD_ROUND;
          m = (msg *) malloc(sizeof(msg));
          m->epoch = epoch;
          m->round = round;
          m->history = history;
          m->history_lenght = history_lenght;
          send(m, leader);
          if (leader == pid)
          {
            memset(mbox_ack_e, 0, sizeof(mbox_ack_e));
            if (num_mbox_ack_e >= (num_processes / 2))
            {
              history = get_longest_history(num_mbox_ack_e);
              history_lenght = lenght(history);
            }
            else
            {
              round = FIRST_ROUND;
              continue;
            }

          }

          round = FORTH_ROUND;
          if (leader == pid)
          {
            m = (msg *) malloc(sizeof(msg));
            m->epoch = epoch;
            m->round = round;
            m->history = history;
            m->history_lenght = history_lenght;
            send(m, to_all);
          }

          memset(mbox_new_l, 0, sizeof(mbox_new_l));
          if (num_mbox_new_l >= 1)
          {
            history = mbox_new_e[num_mbox_new_l].history;
            history_lenght = mbox_new_e[num_mbox_new_l].history_lenght;
          }
          else
          {
            round = FIRST_ROUND;
            continue;
          }

          Synchronization();
        }

      }

    }

  }

}

