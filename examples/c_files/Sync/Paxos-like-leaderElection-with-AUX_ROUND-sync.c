def round NewBallot_ROUND:
  SEND():

if(round == NewBallot_ROUND){
  if ((pid == coord(n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    send(m, to_all);
  }
}

  UPDATE():

if(round == NewBallot_ROUND)
{

  old_0_mbox = mbox;
  if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
  {
    leader = mbox->message->leader;
  }



}


def round AckBallot_ROUND:
  SEND():

if(round == AckBallot_ROUND){
  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->leader = leader;
    send(m, to_all);
  }
}

  UPDATE():

if(round == AckBallot_ROUND)
{
  if ((((old_0_mbox != NULL) && (old_0_mbox->size == 1)) && (old_0_mbox->next == NULL)))
  {
    if (((mbox != NULL) && (mbox->size > (n / 2))) && (all_same(mbox, leader) == 1))
    {
      out(PHASE, leader);
    }
  }
}
