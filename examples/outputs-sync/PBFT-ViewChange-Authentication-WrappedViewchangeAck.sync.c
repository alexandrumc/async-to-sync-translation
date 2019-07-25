No inner algorithm detected

def round ViewChange_ROUND:
  SEND():

if(round == ViewChange_ROUND){
  m = (msg *) malloc(sizeof(msg));
  if ((m == 0))
  {
    abort();
  }
  send(m, to_all);
}

  UPDATE():

if(round == ViewChange_ROUND)
{
  
}


def round ViewChangeAck_ROUND:
  SEND():

if(round == ViewChangeAck_ROUND){
  if ((((old_0_mbox != NULL) && (old_0_mbox->size > ((2 * n) / 3))) && all_same(mbox, view)))
  {
    mviewchange = mbox->message;
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->pid = pid;
    m->pid_ack = list_of_acks(mbox);
    send(m, primary(n, PHASE));
  }
}

  UPDATE():

if(round == ViewChangeAck_ROUND)
{
  if ((((old_0_mbox != NULL) && (old_0_mbox->size > ((2 * n) / 3))) && all_same(mbox, view)))
  {
    mviewchange = mbox->next;
  }
}


def round NewView_ROUND:
  SEND():

if(round == NewView_ROUND){
  if ((primary(n, view) == pid))
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

if(round == NewView_ROUND)
{
  if ((primary(n, view) == pid))
  {
    out(PHASE);
  }
  else
  {
    if (((((((m != NULL) && (mbox->size == 1)) && (mbox->next == NULL)) && (mbox->message->round == NewView_ROUND)) && (view == mbox->message->view)) && certified_newview(m)))
    {
      out(PHASE);
    }
  }
}


