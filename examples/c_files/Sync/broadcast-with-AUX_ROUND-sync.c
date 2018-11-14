def round FIRST_ROUND:
  SEND():

  if ((pid == leader))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->i = i;
    m->round = round;
    m->epoch = epoch;
    m->lab = BCAST;
    m->sender = leader;
    ltype *entry = list_get(log, lastIndex);
    if ((entry != NULL))
    {
      m->op = entry->op;
    }
    send(m, to_all);
  }
  UPDATE():

  if ((pid == leader))
  {
    dispose(m);
  }
  old_0_mbox = mbox;
  if (((mbox != NULL) && (mbox->size >= 1)))
  {
    if (((mbox->message != NULL) && (mbox->message->sender == leader)))
    {
      if ((pid != leader))
      {
        ltype *logi = list_get(log, i);
        if ((logi != 0))
        {
          logi->op = mbox->message->op;
          logi->commit = false;
        }
      }
    }
    else
    {
      out();
    }
    round = SECOND_ROUND;
  }


def round SECOND_ROUND:
  SEND():

  if (((old_0_mbox != NULL) && (old_0_mbox->size >= 1)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->i = i;
    m->round = round;
    m->epoch = epoch;
    m->lab = BCAST;
    m->sender = pid;
    send(m, leader);
  }
  UPDATE():

  if (((old_0_mbox != NULL) && (old_0_mbox->size >= 1)))
  {
    dispose(m);
    old_0_leader = leader;
    if ((pid == leader))
    {
      if (((mbox != NULL) && (mbox->size > (n / 2))))
      {
        ltype *logi = list_get(log, i);
        if ((logi != 0))
        {
          logi->commit = true;
        }
        cmt_number++;
        int size = list_length(log);
        out(logi);
      }
      else
      {
        out();
      }
      round = THIRD_ROUND;
    }
  }
  if (((old_0_mbox != NULL) && (old_0_mbox->size >= 1)))
  {
    dispose(m);
    old_0_leader = leader;
    if (!(pid == leader))
    {
      round = THIRD_ROUND;
    }
  }


def round THIRD_ROUND:
  SEND():

  if (((old_0_mbox != NULL) && (old_0_mbox->size >= 1)))
  {

    if ((pid == leader))
    {
      m = (msg *) malloc(sizeof(msg));
      if ((m == 0))
      {
        abort();
      }
      m->i = i;
      m->round = round;
      m->epoch = epoch;
      m->lab = BCAST;
      m->sender = pid;
      send(m, to_all);
    }
  }
  UPDATE():

  if (((old_0_mbox != NULL) && (old_0_mbox->size >= 1)))
  {
    if ((pid == leader))
    {
      dispose(m);
    }
    if ((mbox != NULL))
    {
    }
    if (((mbox != NULL) && (mbox->size >= 1)))
    {
      if ((pid != leader))
      {
        ltype *logi = list_get(log, i);
        if ((logi != 0))
        {
          logi->commit = true;
        }
        cmt_number++;
        out(logi);
      }
      if ((pid == leader))
      {
        lastIndex++;
        ltype *newEntry = create_ltype(in(), false);
        list_add(log, newEntry);
      }
      else
      {
        lastIndex++;
        ltype *newEntry = create_ltype(-1, false);
        list_add(log, newEntry);
      }
      i++;
      round = FIRST_ROUND;
    }
  }



Process finished with exit code 0
