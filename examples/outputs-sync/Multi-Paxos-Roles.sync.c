Launched procedure for nested algorithms


Algorithm B


def round FIRST_ROUND:
  SEND():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->round = ROUND;
    mB->epoch = epoch;
    mB->lab = BCAST;
    mB->sender = leader;
    ltype *entry = list_get(log, lastIndex);
    if ((entry != NULL))
    {
      mB->op = entry->op;
    }
    send_msgb(mB, to_all);
  }

  UPDATE():

  
  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
  {
    if (((((mboxB != NULL) && (mboxB->size >= 1)) && (mboxB->message != NULL)) && (mboxB->message->sender == leader)))
    {
      ltype *logi = list_get(log, PHASE);
      if ((logi != 0))
      {
        logi->op = mboxB->message->op;
        logi->commit = 0;
      }
    }
    else
    {
      out();
    }
  }


def round SECOND_ROUND:
  SEND():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->round = ROUND;
    mB->epoch = epoch;
    mB->lab = BCAST;
    mB->sender = pid;
    send_msgb(mB, leader);
  }
  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->round = ROUND;
    mB->epoch = epoch;
    mB->lab = BCAST;
    mB->sender = pid;
    send_msgb(mB, leader);
  }

  UPDATE():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    if (((mboxB != NULL) && (mboxB->size > (n / 2))))
    {
      ltype *logi = list_get(log, PHASE);
      if ((logi != 0))
      {
        logi->commit = 1;
      }
      out_external(logi);
    }
    else
    {
      out();
    }
  }
  


def round THIRD_ROUND:
  SEND():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->round = ROUND;
    mB->epoch = epoch;
    mB->lab = BCAST;
    mB->sender = pid;
    send_msgb(mB, to_all);
  }

  UPDATE():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    lastIndex++;
    ltype *newEntry = create_ltype(in(), 0);
    list_add(log, newEntry);
  }
  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
  {
    if (((mboxB != NULL) && (mboxB->size >= 1)))
    {
      ltype *logi = list_get(log, PHASE);
      if ((logi != 0))
      {
        logi->commit = 1;
        out(logi);
      }
    }
    else
    {
      out();
    }
  }


Algorithm A


def round NewEpoch:
  SEND():

  if ((pid == coord(n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->round = NewEpoch;
    m->sender = pid;
    send(m, to_all);
  }

  UPDATE():

  if ((pid == coord(n)))
  {
    round = Ack_E;
  }
  if (!(pid == coord(n)))
  {
    old_1_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      leader = mbox->message->sender;
      round = Ack_E;
    }
  }


def round Ack_E:
  SEND():

  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->round = Ack_E;
    m->history = log;
    m->history_lenght = lastIndex;
    send(m, leader);
  }

  UPDATE():

  if ((pid == coord(n)))
  {
    old_0_mbox = mbox;
    if (((mbox != NULL) && (mbox->size > (n / 2))))
    {
      lastIndex = max_log_size(mbox);
      log = longest_log(mbox, lastIndex);
      round = New_Leader;
    }
  }
  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    round = New_Leader;
  }
  if ((pid == coord(n)))
  {
    old_0_mbox = mbox;
    if (!((mbox != NULL) && (mbox->size > (n / 2))))
    {
      round = NewEpoch;
    }
  }


def round New_Leader:
  SEND():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->round = New_Leader;
    m->history = log;
    m->history_lenght = lastIndex;
    send(m, to_all);
  }

  UPDATE():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    round = BCAST;
  }
  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    old_2_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      lastIndex = mbox->message->history_lenght;
      log = mbox->message->history;
      round = BCAST;
    }
  }
  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    old_2_mbox = mbox;
    if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      round = NewEpoch;
    }
  }


def round BCAST:
  SEND():

  UPDATE():

  if ((pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    int len = list_length(log);
    ltype *lastEntry = list_get(log, lastIndex);
    i = lastIndex;
    if (((lastEntry != NULL) && (lastEntry->commit == 1)))
    {
      i++;
      lastIndex++;
      ltype *newEntry;
      newEntry = create_ltype(in(), 0);
      list_add(log, newEntry);
    }
    bround = FIRST_ROUND;
    inner_algorithm_B(struct arraylist* log, int lastIndex, int leader, list* old_0_mbox, list* old_1_mbox, list* old_2_mbox);
    round = NewEpoch;
  }
  if (!(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
  {
    i = lastIndex;
    ltype *lastEntry = list_get(log, lastIndex);
    if (((lastEntry != NULL) && (lastEntry->commit == 1)))
    {
      i++;
      lastIndex++;
      ltype *newEntry;
      newEntry = create_ltype(-1, 0);
      list_add(log, newEntry);
    }
    bround = FIRST_ROUND;
    inner_algorithm_B(struct arraylist* log, int lastIndex, int leader, list* old_0_mbox, list* old_1_mbox, list* old_2_mbox);
    i++;
    round = NewEpoch;
  }


