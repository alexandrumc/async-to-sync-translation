def round FIRST_ROUND:
  SEND():

  old_phase = phase;
  old_round = round;
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
  leader = leader(phase, n);
  send(m, leader);
  UPDATE():

  dispose(m);
  if ((myid == leader))
  {
    if ((mbox != 0))
    {
    }
    if (((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
    {
      m = max_timestamp(mbox);
      estimate = m->estimate;
      free(m);
    }
    else
    {
    }
  }
  old_round = round;
  round = SECOND_ROUND;


def round SECOND_ROUND:
  SEND():

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
  UPDATE():

  if ((myid == leader))
  {
    dispose(m);
  }
  if (((mbox != 0) && (mbox->message->sender == leader)))
  {
    estimate = mbox->message->estimate;
    timestamp = phase;
  }
  else
  {
  }
  old_round = round;
  round = THIRD_ROUND;


def round THIRD_ROUND:
  SEND():

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
  UPDATE():

  if ((timestamp == phase))
  {
    dispose(m);
  }
  if ((myid == leader))
  {
    if (((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
    {
      ack = 1;
    }
    else
    {
    }
  }
  old_round = round;
  round = FOURTH_ROUND;


def round FOURTH_ROUND:
  SEND():

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
  UPDATE():

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
  old_phase = phase;
  phase = phase + 1;
  round = FIRST_ROUND;



Process finished with exit code 0
