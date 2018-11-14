def round NewBallot_ROUND:
  SEND():

  if ((pid == leader(epoch, n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->epoch = epoch;
    m->round = NewBallot_ROUND;
    send(m, to_all);
  }
  UPDATE():

  if ((pid == leader(epoch, n)))
  {
    free(m);
    reset_timeout();
    old_0_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      epoch = mbox->message->epoch;
      round = AckBallot_ROUND;
    }
  }
  if (!(pid == leader(epoch, n)))
  {
    reset_timeout();
    old_1_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      epoch = mbox->message->epoch;
      round = AckBallot_ROUND;
    }
  }
  if ((pid == leader(epoch, n)))
  {
    free(m);
    reset_timeout();
    old_0_mbox = mbox;
    if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      epoch++;
      round = FIRST_ROUND;
    }
  }
  if (!(pid == leader(epoch, n)))
  {
    reset_timeout();
    old_1_mbox = mbox;
    if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      epoch++;
      round = FIRST_ROUND;
    }
  }


def round AckBallot_ROUND:
  SEND():

  if ((pid == leader(epoch, n)) &&&& (((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->epoch = epoch;
    m->round = AckBallot_ROUND;
    m->history = log;
    m->history_lenght = lastIndex;
    send(m, leader(epoch, n));
  }
  if (!(pid == leader(epoch, n)) &&&& (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->epoch = epoch;
    m->round = AckBallot_ROUND;
    m->history = log;
    m->history_lenght = lastIndex;
    send(m, leader(epoch, n));
  }
  UPDATE():

  if ((pid == leader(epoch, n)) &&&& (((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    free(m);
    if (((mbox != NULL) && (mbox->size > (n / 2))))
    {
      lastIndex = max_log_size(mbox);
      struct arraylist *old_log = log;
      log = longest_log(mbox, lastIndex);
      epoch++;
      round = FIRST_ROUND;
    }
  }
  if ((pid == leader(epoch, n)) &&&& (((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    free(m);
    if (!((mbox != NULL) && (mbox->size > (n / 2))))
    {
      epoch++;
      round = FIRST_ROUND;
    }
  }
  if (!(pid == leader(epoch, n)) &&&& (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    free(m);
    if (((mbox != NULL) && (mbox->size > (n / 2))))
    {
      lastIndex = max_log_size(mbox);
      struct arraylist *old_log = log;
      log = longest_log(mbox, lastIndex);
      epoch++;
      round = FIRST_ROUND;
    }
  }
  if (!(pid == leader(epoch, n)) &&&& (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    free(m);
    if (!((mbox != NULL) && (mbox->size > (n / 2))))
    {
      epoch++;
      round = NewBallot_ROUND;
    }
  }



Process finished with exit code 0
