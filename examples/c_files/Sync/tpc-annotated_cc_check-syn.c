def round FIRST_ROUND:
  SEND():

if(round == FIRST_ROUND)
{
  if ((pid == leader(phase, n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->phase = phase;
    m->round = round;
    m->payload = in();
    m->sender = pid;
    m->response = -1;
    m->commit = -1;
    send(m, to_all);
  }
}

  UPDATE():

if(round == FIRST_ROUND)
{
  if ((pid == leader(phase, n)))
  {
    dispose(m);
  }
  reset_timeout();
  old_0_mbox = mbox;
  if (!((mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    round = ERR_ROUND;
  }
  if ((pid == leader(phase, n)))
  {
    dispose(m);
  }
  reset_timeout();
  old_0_mbox = mbox;
  if (((mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    if ((((mbox->size >= 1) && (mbox->message->sender == leader(phase, n))) && (mbox->message->payload > 0)))
    {
      response = rand_bool();
      current_command = mbox->message->payload;
    }
    round = SECOND_ROUND;
  }
}


def round SECOND_ROUND:
  SEND():

if(round == SECOND_ROUND)
{
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->phase = phase;
    m->round = round;
    m->payload = current_command;
    m->sender = pid;
    m->response = response;
    m->commit = -1;
    send(m, leader(phase, n));
  }
}

  UPDATE():

if(round == SECOND_ROUND)
{
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    dispose(m);
    reset_timeout();
    if ((pid == leader(phase, n)) &&&& !(mbox->size == n))
    {
      round = ERR_ROUND;
    }
  }
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    dispose(m);
    reset_timeout();
    if ((pid == leader(phase, n)))
    {
      if ((mbox->size == n))
      {
        commit = all_agree(mbox);
        round = THIRD_ROUND;
      }
    }
  }
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    dispose(m);
    reset_timeout();
    if (!(pid == leader(phase, n)))
    {
      round = THIRD_ROUND;
    }
  }
}


def round THIRD_ROUND:
  SEND():

if(round == THIRD_ROUND)
{
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    if ((pid == leader(phase, n)))
    {
      m = (msg *) malloc(sizeof(msg));
      if ((m == 0))
      {
        abort();
      }
      m->phase = phase;
      m->round = round;
      m->payload = current_command;
      m->sender = pid;
      m->response = response;
      m->commit = commit;
      send(m, to_all);
    }
  }
}

  UPDATE():

if(round == THIRD_ROUND)
{
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    if ((pid == leader(phase, n)))
    {
      dispose(m);
    }
    reset_timeout();
    old_1_mbox = mbox;
    if (!((mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
    {
      round = ERR_ROUND;
    }
  }
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    if ((pid == leader(phase, n)))
    {
      dispose(m);
    }
    reset_timeout();
    old_1_mbox = mbox;
    if (((mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
    {
      if ((mbox->message->commit == 1))
      {
        out(current_command, pid);
      }
      round = FOURTH_ROUND;
    }
  }
}


def round FOURTH_ROUND:
  SEND():

if(round == FOURTH_ROUND)
{
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))) &&&& ((old_1_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    msg *m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->phase = phase;
    m->round = round;
    m->payload = current_command;
    m->sender = pid;
    m->response = response;
    m->commit = commit;
    send(m, leader(phase, n));
  }
}

  UPDATE():

if(round == FOURTH_ROUND)
{
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))) &&&& ((old_1_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    dispose(m);
    if ((pid == leader(phase, n)))
    {
      reset_timeout();
      if (!(mbox->size == n))
      {
        round = ERR_ROUND;
      }
    }
  }
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))) &&&& ((old_1_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    dispose(m);
    if ((pid == leader(phase, n)))
    {
      reset_timeout();
      if ((mbox->size == n))
      {
        phase = phase + 1;
        round = FIRST_ROUND;
      }
    }
  }
  if (((old_0_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))) &&&& ((old_1_mbox->size >= 1) && (mbox->message->sender == leader(phase, n))))
  {
    dispose(m);
    if (!(pid == leader(phase, n)))
    {
      phase = phase + 1;
      round = FIRST_ROUND;
    }
  }
}



Process finished with exit code 0
