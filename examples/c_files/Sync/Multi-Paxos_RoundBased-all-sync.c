Inner algo code:

def round FIRST_ROUND:
  SEND():

if(round == FIRST_ROUND)
{
  int leader = leader(epoch, n);
  if ((pid == leader(epoch, n)))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->i = i;
    mB->round = bround;
    mB->epoch = epoch;
    mB->lab = Ack_LD;
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

if(round == FIRST_ROUND)
{
  if ((pid == leader(epoch, n)))
  {
    free(mB);
    mB = NULL;
  }
  old_0_mboxB = mboxB;
  if (((((mboxB != NULL) && (mboxB->size >= 1)) && (mboxB->message != NULL)) && (mboxB->message->sender == leader)))
  {
    ltype *logi = list_get(log, i);
    if ((logi != 0))
    {
      logi->op = mboxB->message->op;
      logi->commit = false;
    }
    bround = SECOND_ROUND;
  }
}


def round SECOND_ROUND:
  SEND():

if(round == SECOND_ROUND)
{
  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->i = i;
    mB->round = bround;
    mB->epoch = epoch;
    mB->lab = Ack_LD;
    mB->sender = pid;
    send_msgb(mB, leader);
  }
}

  UPDATE():

if(round == SECOND_ROUND)
{
  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    free(mB);
    mB = NULL;
    listB_dispose_no_data(mboxB);
    if ((pid == leader))
    {
      if (((mboxB != NULL) && (mboxB->size > (n / 2))))
      {
        logi = list_get(log, i);
        if ((logi != 0))
        {
          {
            logi->commit = true;
          }
        }
        out(logi);
      }
      else
      {
        listB_dispose_no_data(mboxB);
        out();
      }
    }
    listB_dispose_no_data(mboxB);
    bround = THIRD_ROUND;
  }
}


def round THIRD_ROUND:
  SEND():

if(round == THIRD_ROUND)
{
  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    mB = (msgb *) malloc(sizeof(msgb));
    if ((mB == 0))
    {
      abort();
    }
    mB->i = i;
    mB->round = bround;
    mB->epoch = epoch;
    mB->lab = Ack_LD;
    mB->sender = pid;
    send_msgb(mB, to_all);
  }
}

  UPDATE():

if(round == THIRD_ROUND)
{
  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    free(mB);
    mB = NULL;
    if (((mboxB != NULL) && (mboxB->size >= 1)))
    {
      logi = list_get(log, i);
      if (((logi != 0) && (pid != leader)))
      {
        logi->commit = true;
        out(logi);
      }
    }
  }
  if (((((old_0_mboxB != NULL) && (old_0_mboxB->size >= 1)) && (old_0_mboxB->message != NULL)) && (old_0_mboxB->message->sender == leader)))
  {
    free(mB);
    mB = NULL;
    if (((mboxB != NULL) && (mboxB->size >= 1)))
    {
      logi = list_get(log, i);
      if (!((logi != 0) && (pid != leader)))
      {
        if ((pid == leader))
        {
          lastIndex++;
          ltype *newEntry = create_ltype(in(), false);
          list_add(log, newEntry);
        }
        bround = FIRST_ROUND;
      }
    }
  }
}



End of inner algo code


Outer Algo code 

def round CEpoch:
  SEND():

  if ((old_0_round == Ack_E))
  {
    if ((pid == leader(epoch, n)) && !((old_1_mbox != NULL) && (old_1_mbox->size > (n / 2))))
    {
    }
    old_1_round = round;
    if ((round == New_Leader))
    {
      if ((pid == leader(epoch, n)))
      {
        m = (msg *) malloc(sizeof(msg));
        if ((m == 0))
        {
          abort();
        }
        m->epoch = epoch;
        m->round = New_Leader;
        m->history = log;
        m->history_lenght = lastIndex;
        send(m, to_all);
      }
    }
  }
  
  old_0_round = round;
  if ((round == Ack_E))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->epoch = epoch;
    m->round = Ack_E;
    m->history = log;
    m->history_lenght = lastIndex;
    send(m, leader(epoch, n));
  }
  UPDATE():

  if ((old_0_round == Ack_E))
  {
    if ((round == New_Leader))
    {
      if ((pid == leader(epoch, n)))
      {
        free(m);
        m = NULL;
      }
      reset_timeout();
      old_0_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        lastIndex = mbox->message->history_lenght;
        log = longest_log(mbox, lastIndex);
        round = Ack_LD;
      }
    }
  }
  if ((round == Ack_E))
  {
    free(m);
    m = NULL;
    if ((pid == leader(epoch, n)))
    {
      old_1_mbox = mbox;
      if (((mbox != NULL) && (mbox->size > (n / 2))))
      {
        lastIndex = max_log_size(mbox);
        log = longest_log(mbox, lastIndex);
        round = New_Leader;
      }
    }
  }
  if ((round == Ack_E))
  {
    free(m);
    m = NULL;
    if (!(pid == leader(epoch, n)))
    {
      round = New_Leader;
    }
  }
  if ((old_0_round == Ack_E))
  {
    if ((round == New_Leader))
    {
      if ((pid == leader(epoch, n)))
      {
        free(m);
        m = NULL;
      }
      reset_timeout();
      old_0_mbox = mbox;
      if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        epoch++;
        round = NewEpoch;
      }
    }
  }


def round NewEpoch:
  SEND():

  if ((pid == leader(epoch, n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->epoch = epoch;
    m->round = NewEpoch;
    send(m, to_all);
  }
  UPDATE():

  if ((pid == leader(epoch, n)))
  {
    free(m);
    m = NULL;
  }
  reset_timeout();
  old_2_mbox = mbox;
  if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
  {
    epoch = mbox->message->epoch;
    round = Ack_E;
  }
  if ((pid == leader(epoch, n)))
  {
    free(m);
    m = NULL;
  }
  reset_timeout();
  old_2_mbox = mbox;
  if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
  {
    epoch++;
    round = CEpoch;
  }


def round Ack_E:
  SEND():

  
  old_0_round = round;
  if ((round == Ack_E))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->epoch = epoch;
    m->round = Ack_E;
    m->history = log;
    m->history_lenght = lastIndex;
    send(m, leader(epoch, n));
  }
  UPDATE():

  if ((round == Ack_E))
  {
    free(m);
    m = NULL;
    if ((pid == leader(epoch, n)))
    {
      old_1_mbox = mbox;
      if (((mbox != NULL) && (mbox->size > (n / 2))))
      {
        lastIndex = max_log_size(mbox);
        log = longest_log(mbox, lastIndex);
        round = New_Leader;
      }
    }
  }
  if ((round == Ack_E))
  {
    free(m);
    m = NULL;
    if (!(pid == leader(epoch, n)))
    {
      round = New_Leader;
    }
  }
  if ((round == Ack_E))
  {
    free(m);
    m = NULL;
    if ((pid == leader(epoch, n)))
    {
      old_1_mbox = mbox;
      if (!((mbox != NULL) && (mbox->size > (n / 2))))
      {
        epoch++;
        round = CEpoch;
      }
    }
  }


def round New_Leader:
  SEND():

  if ((old_0_round == Ack_E))
  {
    if ((pid == leader(epoch, n)) && ((old_1_mbox != NULL) && (old_1_mbox->size > (n / 2))))
    {
    }
    old_1_round = round;
    if ((round == New_Leader))
    {
      if ((pid == leader(epoch, n)))
      {
        m = (msg *) malloc(sizeof(msg));
        if ((m == 0))
        {
          abort();
        }
        m->epoch = epoch;
        m->round = New_Leader;
        m->history = log;
        m->history_lenght = lastIndex;
        send(m, to_all);
      }
    }
  }
  if ((old_0_round == Ack_E))
  {
    
    old_1_round = round;
    if ((round == New_Leader))
    {
      if ((pid == leader(epoch, n)))
      {
        m = (msg *) malloc(sizeof(msg));
        if ((m == 0))
        {
          abort();
        }
        m->epoch = epoch;
        m->round = New_Leader;
        m->history = log;
        m->history_lenght = lastIndex;
        send(m, to_all);
      }
    }
  }
  UPDATE():

  if ((old_0_round == Ack_E))
  {
    if ((round == New_Leader))
    {
      if ((pid == leader(epoch, n)))
      {
        free(m);
        m = NULL;
      }
      reset_timeout();
      old_0_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        lastIndex = mbox->message->history_lenght;
        log = longest_log(mbox, lastIndex);
        round = Ack_LD;
      }
    }
  }
  if ((old_0_round == Ack_E))
  {
    if ((round == New_Leader))
    {
      if ((pid == leader(epoch, n)))
      {
        free(m);
        m = NULL;
      }
      reset_timeout();
      old_0_mbox = mbox;
      if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        epoch++;
        round = NewEpoch;
      }
    }
  }


def round Ack_LD:
  SEND():

  UPDATE():

  if ((old_0_round == Ack_E) && (old_1_round == New_Leader))
  {
    
    if ((round == Ack_LD))
    {
      listb *mboxB = NULL;
      listb *mboxB_new = NULL;
      msgb *mB = NULL;
      int len = list_length(log);
      ltype *lastEntry = list_get(log, lastIndex);
      int i = lastIndex;
      if (((lastEntry != NULL) && (lastEntry->commit == true)))
      {
        i++;
        lastIndex++;
        ltype *newEntry;
        newEntry = create_ltype(-1, false);
        list_add(log, newEntry);
      }
      enum round_typ_B bround = FIRST_ROUND;
      inner_algorithm();
      epoch++;
      round = NewEpoch;
    }
  }



Process finished with exit code 0
