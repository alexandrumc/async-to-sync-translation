Launched procedure for nested algorithms


Algorithm B


def round FIRST_ROUND:
  SEND():

  if ((iter == PHASE))
  {
    if ((!b0))
    {
      if ((logi != 0))
      {
        logi->commit = 1;
        out(logi);
      }
      b0 = True;
    }
    if ((bround == FIRST_ROUND) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
    {
      mB = (msgb *) malloc(sizeof(msgb));
      if ((mB == 0))
      {
        abort();
      }
      mB->round = bround;
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
  }

  UPDATE():

  if ((iter == PHASE))
  {
    if ((bround == FIRST_ROUND) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
    {
      bround = SECOND_ROUND;
    }
  }
  if ((iter == PHASE) && (bround == FIRST_ROUND) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
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
    bround = SECOND_ROUND;
  }


def round SECOND_ROUND:
  SEND():

  if ((iter == PHASE) && (bround == SECOND_ROUND) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->round = bround;
    mB->epoch = epoch;
    mB->lab = BCAST;
    mB->sender = pid;
    send_msgb(mB, leader);
  }
  if ((iter == PHASE) && (bround == SECOND_ROUND) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->round = bround;
    mB->epoch = epoch;
    mB->lab = BCAST;
    mB->sender = pid;
    send_msgb(mB, leader);
  }

  UPDATE():

  if ((iter == PHASE) && (bround == SECOND_ROUND) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
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
    bround = THIRD_ROUND;
  }
  if ((iter == PHASE) && (bround == SECOND_ROUND) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
  {
    bround = THIRD_ROUND;
  }


def round THIRD_ROUND:
  SEND():

  if ((iter == PHASE) && (bround == THIRD_ROUND) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->round = bround;
    mB->epoch = epoch;
    mB->lab = BCAST;
    mB->sender = pid;
    send_msgb(mB, to_all);
  }

  UPDATE():

  if ((iter == PHASE) && (bround == THIRD_ROUND) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    lastIndex++;
    ltype *newEntry = create_ltype(in(), 0);
    list_add(log, newEntry);
    bround = FIRST_ROUND;
  }
  if ((iter == PHASE) && (bround == THIRD_ROUND) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
  {
    if (((mboxB != NULL) && (mboxB->size >= 1)))
    {
      ltype *logi = list_get(log, PHASE);
      PHASE = mboxB->message->i;
      if ((iter == PHASE))
      {
        if ((logi != 0))
        {
          logi->commit = 1;
          out(logi);
        }
        b0 = False;
      }
      else
      {
        b0 = True;
      }
    }
    else
    {
      out();
    }
    bround = FIRST_ROUND;
  }


Algorithm A


def round NewEpoch:
  SEND():

  if ((iter == PHASE))
  {
    if ((!b0))
    {
      leader = mbox->message->sender;
      b0 = True;
    }
    if ((round == NewEpoch) && (pid == coord(n)))
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
  }

  UPDATE():

  if ((iter == PHASE))
  {
    if ((round == NewEpoch) && (pid == coord(n)))
    {
      round = Ack_E;
    }
  }
  if ((iter == PHASE) && (round == NewEpoch) && !(pid == coord(n)))
  {
    old_1_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      PHASE = mbox->message->epoch;
      if ((iter == PHASE))
      {
        leader = mbox->message->sender;
        round = Ack_E;
        b0 = False;
      }
      else
      {
        b0 = True;
      }
    }
  }


def round Ack_E:
  SEND():

  if ((iter == PHASE) && (round == Ack_E) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
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

  if ((iter == PHASE) && (round == Ack_E) && (pid == coord(n)))
  {
    old_0_mbox = mbox;
    if (((mbox != NULL) && (mbox->size > (n / 2))))
    {
      lastIndex = max_log_size(mbox);
      log = longest_log(mbox, lastIndex);
      round = New_Leader;
    }
  }
  if ((iter == PHASE) && (round == Ack_E) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    round = New_Leader;
  }
  if ((iter == PHASE) && (round == Ack_E) && (pid == coord(n)))
  {
    old_0_mbox = mbox;
    if (!((mbox != NULL) && (mbox->size > (n / 2))))
    {
      PHASE++;
      round = NewEpoch;
    }
  }


def round New_Leader:
  SEND():

  if ((iter == PHASE) && (round == New_Leader) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
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

  if ((iter == PHASE) && (round == New_Leader) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
  {
    round = BCAST;
  }
  if ((iter == PHASE) && (round == New_Leader) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    old_2_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      lastIndex = mbox->message->history_lenght;
      log = mbox->message->history;
      round = BCAST;
    }
  }
  if ((iter == PHASE) && (round == New_Leader) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    old_2_mbox = mbox;
    if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
    {
      PHASE++;
      round = NewEpoch;
    }
  }


def round BCAST:
  SEND():

  UPDATE():

  if ((iter == PHASE) && (round == BCAST) && (pid == coord(n)) && ((old_0_mbox != NULL) && (old_0_mbox->size > (n / 2))))
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
    PHASE++;
    round = NewEpoch;
  }
  if ((iter == PHASE) && (round == BCAST) && !(pid == coord(n)) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)) && (((old_2_mbox != NULL) && (old_2_mbox->size == 1)) && (old_2_mbox->next == NULL)))
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
    PHASE++;
    round = NewEpoch;
  }
