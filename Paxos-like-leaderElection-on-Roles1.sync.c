No inner algorithm detected

def round NewBallot_ROUND:
  SEND():

  if ((pid == coord(n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->leader = pid;
    send(m, to_all);
  }

  UPDATE():

  if ((pid == coord(n)))
  {
    if ((iter == ballot))
    {
      old_0_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        leader = mbox->message->leader;
      }
    }
  }
  if (!(pid == coord(n)))
  {
    if ((iter == ballot))
    {
      old_1_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        leader = mbox->message->leader;
      }
    }
  }


def round AckBallot_ROUND:
  SEND():

  if ((pid == coord(n)) && (iter == ballot) && (((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->leader = leader;
    send(m, to_all);
  }
  if (!(pid == coord(n)) && (iter == ballot) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->leader = leader;
    send(m, to_all);
  }

  UPDATE():

  if ((pid == coord(n)) && (iter == ballot) && (((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if (((mbox != NULL) && (mbox->size > (n / 2))) && (all_same(mbox, leader) == 1))
    {
      out(PHASE, leader);
    }
  }
  if (!(pid == coord(n)) && (iter == ballot) && (((old_1_mbox != NULL) && (old_1_mbox->size == 1)) && (old_1_mbox->next == NULL)))
  {
    if (((mbox != NULL) && (mbox->size > (n / 2))) && (all_same(mbox, leader) == 1))
    {
      out(PHASE, leader);
    }
  }


