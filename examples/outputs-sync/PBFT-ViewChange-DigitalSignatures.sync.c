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

  


def round NewView_ROUND:
  SEND():

  
  if (((mbox->size > ((2 * n) / 3)) && all_same(mbox, view)))
  {
    if ((primary(n, view) == pid))
    {
      printf("I am %i, the primary of the new view\n", pid);
      m = (msg *) malloc(sizeof(msg));
      if ((m == 0))
      {
        abort();
      }
      send(m, to_all);
    }
    else
  }

  UPDATE():

  if (((mbox->size > ((2 * n) / 3)) && all_same(mbox, view)))
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


