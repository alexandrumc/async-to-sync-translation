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

  if ((((old_0_mbox != NULL) && (old_0_mbox->size > ((2 * n) / 3))) && all_same(mbox, view)))
  {
    while (true)
    {
      mviewchange = mbox->message;
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

  if ((((old_0_mbox != NULL) && (old_0_mbox->size > ((2 * n) / 3))) && all_same(mbox, view)))
  {
    while (true)
    {
      mviewchange = mbox->next;
      if ((mviewchange == NULL))
      {
        out();
      }
    }
  }


def round NewView_ROUND:
  SEND():

  UPDATE():



