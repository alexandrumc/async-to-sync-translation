No inner algorithm detected

def round StartViewChange_ROUND:
  SEND():

if(round == StartViewChange_ROUND){
  if ((pid == get_primary(view_nr, n)))
  {
    msg = malloc(sizeof(msg_ViewChange *));
    if ((!msg))
    {
      abort();
    }
    msg->replica_id = pid;
    msg->label = StartViewChange;
    send((void *) msg, to_all);
  }
  if (!(pid == get_primary(view_nr, n)))
  {
    msg = malloc(sizeof(msg_ViewChange *));
    if ((!msg))
    {
      abort();
    }
    msg->replica_id = pid;
    msg->label = StartViewChange;
    send((void *) msg, to_all);
  }
}

  UPDATE():

if(round == StartViewChange_ROUND)
{
  if ((pid == get_primary(view_nr, n)))
  {
    
    old_0_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size >= (n / 2))) && (mbox->next == NULL)))
    {
      round = DoViewChange_ROUND;
    }
  }
  if (!(pid == get_primary(view_nr, n)))
  {
    
    old_2_mbox = mbox;
    if ((((mbox != NULL) && (mbox->size >= (n / 2))) && (mbox->next == NULL)))
    {
      round = DoViewChange_ROUND;
    }
  }
  if ((pid == get_primary(view_nr, n)))
  {
    old_0_msg = msg;
    if (!(((msg != NULL) && (msg->view_nr > view_nr)) && ((msg->label == StartViewChange) || (msg->label == DoViewChange))))
    {
      old_0_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size >= (n / 2))) && (mbox->next == NULL)))
      {
        round = DoViewChange_ROUND;
      }
    }
  }
  if (!(pid == get_primary(view_nr, n)))
  {
    old_1_msg = msg;
    if (!(((msg != NULL) && (msg->view_nr > view_nr)) && ((msg->label == StartViewChange) || (msg->label == DoViewChange))))
    {
      old_2_mbox = mbox;
      if ((((mbox != NULL) && (mbox->size >= (n / 2))) && (mbox->next == NULL)))
      {
        round = DoViewChange_ROUND;
      }
    }
  }
}


def round DoViewChange_ROUND:
  SEND():

if(round == DoViewChange_ROUND){
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mbox != NULL) && (old_2_mbox->size >= (n / 2))) && (old_2_mbox->next == NULL)))
  {
    msg = malloc(sizeof(msg_ViewChange));
    if ((!msg))
    {
      abort();
    }
    msg->replica_id = pid;
    msg->label = DoViewChange;
    msg->log_size = log->size;
    msg->log = log->array;
    msg->prev_view = prev_view;
    send(msg, get_primary(PHASE, pid));
  }
}

  UPDATE():

if(round == DoViewChange_ROUND)
{
  if ((pid == get_primary(view_nr, n)) && (((old_0_mbox != NULL) && (old_0_mbox->size >= (n / 2))) && (old_0_mbox->next == NULL)))
  {
    old_1_mbox = mbox;
    if (((mbox != NULL) && (mbox->size >= (n / 2))))
    {
      round = StartView_ROUND;
    }
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mbox != NULL) && (old_2_mbox->size >= (n / 2))) && (old_2_mbox->next == NULL)))
  {
    round = StartView_ROUND;
  }
  if ((pid == get_primary(view_nr, n)) && (((old_0_mbox != NULL) && (old_0_mbox->size >= (n / 2))) && (old_0_mbox->next == NULL)))
  {
    old_1_mbox = mbox;
    if (!((mbox != NULL) && (mbox->size >= (n / 2))))
    {
      round = StartViewChange_ROUND;
    }
  }
}


def round StartView_ROUND:
  SEND():

if(round == StartView_ROUND){
  if ((pid == get_primary(view_nr, n)))
  {
    if ((((old_0_mbox != NULL) && (old_0_mbox->size >= (n / 2))) && (old_0_mbox->next == NULL)) && ((old_1_mbox != NULL) && (old_1_mbox->size >= (n / 2))))
    {
      msg = malloc(sizeof(msg_ViewChange));
      if ((!msg))
      {
        abort();
      }
      msg->label = StartView;
      msg->log_size = log->size;
      msg->log = log->array;
      send((void *) msg, to_all);
    }
  }
}

  UPDATE():

if(round == StartView_ROUND)
{
  if ((pid == get_primary(view_nr, n)))
  {
    
    round = StartViewChange_ROUND;
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mbox != NULL) && (old_2_mbox->size >= (n / 2))) && (old_2_mbox->next == NULL)))
  {
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && ((msg->label == StartViewChange) || (msg->label == DoViewChange))))
    {
      round = StartViewChange_ROUND;
    }
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mbox != NULL) && (old_2_mbox->size >= (n / 2))) && (old_2_mbox->next == NULL)))
  {
    if (!(((msg != NULL) && (msg->view_nr > view_nr)) && ((msg->label == StartViewChange) || (msg->label == DoViewChange))))
    {
      round = StartViewChange_ROUND;
    }
  }
}


