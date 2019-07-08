No inner algorithm detected

def round ViewChange_ROUND:
  SEND():

  m = (msg *) malloc(sizeof(msg));
  if ((m == 0))
  {
    abort();
  }
  send(m, to_all);

  UPDATE():




def round ViewChangeAck_ROUND:
  SEND():

  if ((((mbox != NULL) && (mbox->size > ((2 * n) / 3))) && all_same(mbox, view)))
  {
    view = mbox->message->view;
    msg *mviewchange = mbox->message;
    while (mviewchange != NULL)
    {
      m = (msg *) malloc(sizeof(msg));
      if ((m == 0))
      {
        abort();
      }
      m->pid = pid;
      m->pid_ack = mviewchange->pid;
      send(m, primary(n, PHASE));
    }
  }

  UPDATE():

  if ((((mbox != NULL) && (mbox->size > ((2 * n) / 3))) && all_same(mbox, view)))
  {
    while ()
    {
      dispose(m);
      mviewchange = mbox->next->message;
    }
  }


def round NewView_ROUND:
  SEND():

  if ((primary(n, view) == pid))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    send(m, to_all);
  }
  else

  UPDATE():

  if ((primary(n, view) == pid))
  {
    out(PHASE);
  }
  else
  {
    if ((((((m != NULL) && (mbox->size == 1)) && (mbox->next == NULL)) && (mbox->message->round == NewView_ROUND)) && (view == mbox->message->view)))
    {
      out(PHASE);
    }
  }


def round Echo_ROUND:
  SEND():

  m = (msg *) malloc(sizeof(msg));
  if ((m == 0))
  {
    abort();
  }
  send(m, to_all);

  UPDATE():




