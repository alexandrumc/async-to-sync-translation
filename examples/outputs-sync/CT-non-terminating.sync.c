No inner algorithm detected

def round FIRST_ROUND:
  SEND():

  m = (msg *) malloc(sizeof(msg));
  if ((m == 0))
  {
    abort();
  }
  m->estimate = estimate;
  m->sender = myid;
  m->timestamp = timestamp;
  leader = leader(PHASE, n);
  send(m, leader);

  UPDATE():

  if ((myid == leader) && ((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
  {
    m = max_timestamp(mbox);
    estimate = m->estimate;
  }


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
    m->estimate = estimate;
    m->timestamp = timestamp;
    send(m, to_all);
  }

  UPDATE():

  
  if (((mbox != 0) && (mbox->message->sender == leader)))
  {
    estimate = mbox->message->estimate;
    timestamp = PHASE;
  }


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
    m->estimate = estimate;
    m->timestamp = timestamp;
    send(m, leader);
  }

  UPDATE():

  
  if ((myid == leader) && ((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
  {
    ack = 1;
  }


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
    m->estimate = estimate;
    m->ack = ack;
    send(m, to_all);
  }

  UPDATE():

  
  if ((mbox != 0))
  {
    estimate = mbox->message->estimate;
    state = 1;
    out(myid, estimate);
  }


