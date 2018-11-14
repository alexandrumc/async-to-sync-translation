def round NEW_BALLOT_ROUND:
  SEND():

if(round == NEW_BALLOT_ROUND)
{
  if ((pid == leader(ballot, n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->ballot = ballot;
    m->round = NEW_BALLOT_ROUND;
    send(m, to_all);
  }
}

  UPDATE():

if(round == NEW_BALLOT_ROUND)
{
  if ((pid == leader(ballot, n)))
  {
    dispose(m);
    m = NULL;
  }
  reset_timeout();
  old_0_mbox = mbox;
  if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
  {
    ballot = mbox->message->ballot;
    round = ACK_BALLOT_ROUND;
  }
  if ((pid == leader(ballot, n)))
  {
    dispose(m);
    m = NULL;
  }
  reset_timeout();
  old_0_mbox = mbox;
  if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
  {
    ballot++;
    round = FIRST_ROUND;
  }
}


def round ACK_BALLOT_ROUND:
  SEND():

if(round == ACK_BALLOT_ROUND)
{
  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->ballot = ballot;
    m->round = ACK_BALLOT_ROUND;
    send(m, to_all);
  }
}

  UPDATE():

if(round == ACK_BALLOT_ROUND)
{
  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    dispose(m);
    m = NULL;
    list_dispose1(mbox);
    reset_timeout();
    if (((mbox != NULL) && (mbox->size > (n / 2))))
    {
      printf("\n%d", ballot);
      int leader = leader(ballot, n);
      out(ballot, leader);
    }
    if ((mbox != 0))
    {
      list_dispose(mbox);
    }
  }
  ballot++;
  round = FIRST_ROUND;
}



Process finished with exit code 0
