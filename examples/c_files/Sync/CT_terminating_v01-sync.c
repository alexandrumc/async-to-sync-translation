def round FIRST_ROUND:
  SEND():

if(round == FIRST_ROUND)
{
  leader = leader(phase, n);
  m = (msg *) malloc(sizeof(msg));
  if ((m == 0))
  {
    abort();
  }
  m->phase = phase;
  m->round = round;
  m->estimate = estimate;
  m->sender = myid;
  m->timestamp = timestamp;
  m->ack = 0;
  send(m, leader);
}

  UPDATE():

if(round == FIRST_ROUND)
{
  dispose(m);
  old_0_myid = myid;
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if (!(mbox->size >= ((n + 1) / 2)))
    {
      round = ERR_ROUND;
    }
  }
  dispose(m);
  old_0_myid = myid;
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if ((mbox->size >= ((n + 1) / 2)))
    {
      old_1_mbox = mbox;
      if (((mbox != 0) && (mbox->message != NULL)) && (jump == True))
      {
        round = FOURTH_ROUND;
      }
    }
  }
  dispose(m);
  old_0_myid = myid;
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if ((mbox->size >= ((n + 1) / 2)))
    {
      old_1_mbox = mbox;
      if (((mbox != 0) && (mbox->message != NULL)))
      {
        if (!(jump == True))
        {
          if (((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
          {
            m = max_timestamp(mbox);
            estimate = m->estimate;
            free(m);
          }
          round = SECOND_ROUND;
        }
      }
    }
  }
  dispose(m);
  old_0_myid = myid;
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if ((mbox->size >= ((n + 1) / 2)))
    {
      old_1_mbox = mbox;
      if (!((mbox != 0) && (mbox->message != NULL)))
      {
        round = SECOND_ROUND;
      }
    }
  }
  dispose(m);
  old_0_myid = myid;
  if (!(myid == leader))
  {
    round = SECOND_ROUND;
  }
}


def round FOURTH_ROUND:
  SEND():

if(round == FOURTH_ROUND)
{
  if (((myid == leader) && (ack == 1)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->sender = myid;
    m->phase = phase;
    m->round = round;
    m->estimate = estimate;
    m->ack = ack;
    send(m, to_all);
  }
}

  UPDATE():

if(round == FOURTH_ROUND)
{
  if (((old_2_mbox != 0) && (old_2_mbox->message != NULL)) && (mbox->message->round == FOURTH_ROUND))
  {
    estimate = mbox->message->estimate;
    state = 1;
    out();
  }
  round = THIRD_ROUND;
  if ((old_0_myid == leader) && (old_0_mbox->size >= ((n + 1) / 2)) && ((old_1_mbox != 0) && (old_1_mbox->message != NULL)) && (mbox->message->round == FOURTH_ROUND))
  {
    estimate = mbox->message->estimate;
    state = 1;
    out();
  }
  round = SECOND_ROUND;
  if (((myid == leader) && (ack == 1)))
  {
    dispose(m);
  }
  if ((mbox != 0))
  {
    estimate = mbox->message->estimate;
    state = 1;
    out(myid, estimate);
  }
  phase = phase + 1;
  round = FIRST_ROUND;
}


def round SECOND_ROUND:
  SEND():

if(round == SECOND_ROUND)
{
  if ((myid == leader))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->sender = myid;
    m->phase = phase;
    m->round = round;
    m->estimate = estimate;
    m->timestamp = timestamp;
    send(m, to_all);
  }
}

  UPDATE():

if(round == SECOND_ROUND)
{
  if ((myid == leader))
  {
    dispose(m);
  }
  if (((mbox != 0) && (mbox->message != NULL)) && (jump == True))
  {
    round = FOURTH_ROUND;
  }
  if ((myid == leader))
  {
    dispose(m);
  }
  if (((mbox != 0) && (mbox->message != NULL)))
  {
    if (!(jump == True))
    {
      if ((mbox != 0))
      {
        estimate = mbox->message->estimate;
        timestamp = phase;
      }
      round = THIRD_ROUND;
    }
  }
  if ((myid == leader))
  {
    dispose(m);
  }
  if (!((mbox != 0) && (mbox->message != NULL)))
  {
    round = THIRD_ROUND;
  }
}


def round THIRD_ROUND:
  SEND():

if(round == THIRD_ROUND)
{
  if ((timestamp == phase))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->sender = myid;
    m->phase = phase;
    m->round = round;
    m->estimate = estimate;
    m->timestamp = timestamp;
    send(m, leader);
  }
}

  UPDATE():

if(round == THIRD_ROUND)
{
  if ((timestamp == phase))
  {
    dispose(m);
  }
  if ((myid == leader) && ((mbox != 0) && (mbox->message != NULL)) && (jump == True))
  {
    round = FOURTH_ROUND;
  }
  if ((timestamp == phase))
  {
    dispose(m);
  }
  if ((myid == leader) && ((mbox != 0) && (mbox->message != NULL)))
  {
    if (!(jump == True))
    {
      if (((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
      {
        ack = 1;
      }
      round = FOURTH_ROUND;
    }
  }
  if ((timestamp == phase))
  {
    dispose(m);
  }
  if ((myid == leader) && !((mbox != 0) && (mbox->message != NULL)))
  {
    round = FOURTH_ROUND;
  }
  if ((timestamp == phase))
  {
    dispose(m);
  }
  if (!(myid == leader))
  {
    round = FOURTH_ROUND;
  }
}

