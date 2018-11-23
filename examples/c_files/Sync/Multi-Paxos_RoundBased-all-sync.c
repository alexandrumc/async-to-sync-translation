Inner algo code:

def round FIRST_ROUND:
  SEND():

  if ((pid == leader))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->i = i;
    mB->round = bround;
    mB->lab = Ack_LD;
    mB->sender = pid;
    ltype *entry = list_get(log, lastIndex);
    if ((entry != NULL))
    {
      mB->op = entry->op;
    }
    send_msgb(mB, to_all);
  }

  UPDATE():


  old_0_mboxB = mboxB;
  if (((((mboxB != NULL) && (mboxB->size >= 1)) && (mboxB->message != NULL)) && (mboxB->message->sender == leader)))
  {
    ltype *logi = list_get(log, i);
    if ((logi != 0))
    {
      logi->op = mboxB->message->op;
      logi->commit = 0;
    }
    bround = SECOND_ROUND;
  }


def round SECOND_ROUND:
  SEND():

  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->i = i;
    mB->round = bround;
    mB->lab = Ack_LD;
    mB->sender = pid;
    send_msgb(mB, leader);
  }

  UPDATE():

  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    if ((pid == leader))
    {
      if (((mboxB != NULL) && (mboxB->size > (n / 2))))
      {
        logi = list_get(log, i);
        if ((logi != 0))
        {
          logi->commit = 1;
        }
        out(logi);
      }
      else
      {
        out();
      }
    }
    bround = THIRD_ROUND;
  }


def round THIRD_ROUND:
  SEND():

  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    if ((pid == leader))
    {
      mB = (msgb *) malloc(sizeof(msgb));
      if ((mB == 0))
      {
        abort();
      }
      mB->i = i;
      mB->round = bround;
      mB->lab = Ack_LD;
      mB->sender = pid;
      send_msgb(mB, to_all);
    }
  }

  UPDATE():

  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {

    if (((mboxB != NULL) && (mboxB->size >= 1)))
    {
      logi = list_get(log, i);
      if (((logi != 0) && (pid != leader)))
      {
        logi->commit = 1;
        out(logi);
      }
      if ((pid == leader))
      {
        lastIndex++;
        ltype *newEntry = create_ltype(in(), 0);
        list_add(log, newEntry);
      }
      bround = FIRST_ROUND;
    }
  }



End of inner algo code


Outer Algo code

def round NewEpoch:
  SEND():

  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {

    old_0_round = round;
    if ((round == New_Leader))
    {
      if ((pid == coord))
      {
        m = (msg *) malloc(sizeof(msg));
        if ((m == 0))
        {
          abort();
        }
        m->round = New_Leader;
        m->history = log;
        m->history_lenght = lastIndex;
        m->sender = pid;
        send(m, to_all);
      }
    }
  }
  if ((pid == coord()))
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
  else

  UPDATE():

  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if ((round == New_Leader))
    {

      old_1_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        lastIndex = mbox->message->history_lenght;
        log = longest_log(mbox, lastIndex);
        round = Ack_LD;
      }
    }
  }
  if ((pid == coord()))
  {
    coord = pid;
  }
  else
  {
    coord = -1;
  }
  old_0_mbox = mbox;
  if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
  {
    leader = mbox->message->sender;
    round = Ack_E;
  }


def round Ack_E:
  SEND():

  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
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

  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if ((pid == coord))
    {
      old_2_mbox = mbox;
      if (((mbox != NULL) && (mbox->size > (n / 2))))
      {
        lastIndex = max_log_size(mbox);
        log = longest_log(mbox, lastIndex);
        round = New_Leader;
      }
    }
  }
  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if (!(pid == coord))
    {
      round = New_Leader;
    }
  }
  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if ((pid == coord))
    {
      old_2_mbox = mbox;
      if (!((mbox != NULL) && (mbox->size > (n / 2))))
      {
        round = NewEpoch;
      }
    }
  }


def round New_Leader:
  SEND():

  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {

    old_0_round = round;
    if ((round == New_Leader))
    {
      if ((pid == coord))
      {
        m = (msg *) malloc(sizeof(msg));
        if ((m == 0))
        {
          abort();
        }
        m->round = New_Leader;
        m->history = log;
        m->history_lenght = lastIndex;
        m->sender = pid;
        send(m, to_all);
      }
    }
  }

  UPDATE():

  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if ((round == New_Leader))
    {

      old_1_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        lastIndex = mbox->message->history_lenght;
        log = longest_log(mbox, lastIndex);
        round = Ack_LD;
      }
    }
  }
  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if ((round == New_Leader))
    {

      old_1_mbox = mbox;
      if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        round = NewEpoch;
      }
    }
  }


def round Ack_LD:
  SEND():

  UPDATE():

  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)) && (old_0_round == New_Leader))
  {

    if ((round == Ack_LD))
    {
      listb *mboxB = NULL;
      listb *mboxB_new = NULL;
      msgb *mB = NULL;
      int len = list_length(log);
      ltype *lastEntry = list_get(log, lastIndex);
      int i = lastIndex;
      if (((lastEntry != NULL) && (lastEntry->commit == 1)))
      {
        i++;
        lastIndex++;
        ltype *newEntry;
        if ((pid == leader))
        {
          newEntry = create_ltype(in(), 0);
        }
        else
        {
          newEntry = create_ltype(-1, 0);
        }
        list_add(log, newEntry);
      }
      enum round_typ_B baFIRST_ROUND;
      inner_algorithm();
      round = NewEpoch;
    }
  }
