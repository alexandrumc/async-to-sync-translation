No inner algorithm detected

def round StartViewChange_ROUND:
  SEND():

if(round == StartViewChange_ROUND){
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
}

  UPDATE():

if(round == StartViewChange_ROUND)
{
  if (!(pid == get_primary(view_nr, n)))
  {
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == StartViewChange)))
    {
      continue;
    }
    old_1_msg = msg;
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == DoViewChange)))
    {
      int no_recvs = (n / 2) - 1;
      old_2_mbox = mbox;
      if (((mbox != NULL) && (mbox->size >= no_recvs)))
      {
        int Max;
        log->array = choose_log(mbox, &Max);
        log->size = (log->current = Max);
        round = StartView_ROUND;
      }
    }
  }
  if ((pid == get_primary(view_nr, n)))
  {
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == StartViewChange)))
    {
      continue;
    }
    old_0_mbox = mbox;
    old_0_msg = msg;
    if (((((mbox != NULL) && (mbox->size >= (n / 2))) && (mbox->next == NULL)) || (((msg != NULL) && (msg->view_nr >= view_nr)) && (jump == True))))
    {
      int no_recvs = n / 2;
      if (((msg != NULL) && (jump == True)))
      {
        no_recvs--;
        
      }
      round = DoViewChange_ROUND;
    }
  }
  if (!(pid == get_primary(view_nr, n)))
  {
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == StartViewChange)))
    {
      continue;
    }
    old_1_msg = msg;
    if (!(((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == DoViewChange)))
    {
      old_3_mbox = mbox;
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
  if (!(pid == get_primary(view_nr, n)) && !(((old_1_msg != NULL) && (old_1_msg->view_nr > view_nr)) && (old_1_msg->label == DoViewChange)) && (((old_3_mbox != NULL) && (old_3_mbox->size >= (n / 2))) && (old_3_mbox->next == NULL)))
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
  
  if (!(pid == get_primary(view_nr, n)) && !(((old_1_msg != NULL) && (old_1_msg->view_nr > view_nr)) && (old_1_msg->label == DoViewChange)) && (((old_3_mbox != NULL) && (old_3_mbox->size >= (n / 2))) && (old_3_mbox->next == NULL)))
  {
    round = StartView_ROUND;
  }
}


def round StartView_ROUND:
  SEND():

if(round == StartView_ROUND){
  if ((pid == get_primary(view_nr, n)))
  {
    if (((((old_0_mbox != NULL) && (old_0_mbox->size >= (n / 2))) && (old_0_mbox->next == NULL)) || (((old_0_msg != NULL) && (old_0_msg->view_nr >= view_nr)) && (jump == True))) && ((old_1_mbox != NULL) && (old_1_mbox->size >= no_recvs)))
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
  if (!(pid == get_primary(view_nr, n)))
  {
    if ((((old_1_msg != NULL) && (old_1_msg->view_nr > view_nr)) && (old_1_msg->label == DoViewChange)) && ((old_2_mbox != NULL) && (old_2_mbox->size >= no_recvs)))
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
  if (!(pid == get_primary(view_nr, n)))
  {
    if (!(((old_1_msg != NULL) && (old_1_msg->view_nr > view_nr)) && (old_1_msg->label == DoViewChange)) && (((old_3_mbox != NULL) && (old_3_mbox->size >= (n / 2))) && (old_3_mbox->next == NULL)) && (((old_2_msg != NULL) && (old_2_msg->view_nr > view_nr)) && (old_2_msg->label == DoViewChange)) && ((old_4_mbox != NULL) && (old_4_mbox->size >= no_recvs)))
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
  if (!(pid == get_primary(view_nr, n)))
  {
    
    round = StartViewChange_ROUND;
  }
  if (!(pid == get_primary(view_nr, n)) && !(((old_1_msg != NULL) && (old_1_msg->view_nr > view_nr)) && (old_1_msg->label == DoViewChange)) && (((old_3_mbox != NULL) && (old_3_mbox->size >= (n / 2))) && (old_3_mbox->next == NULL)))
  {
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == StartViewChange)))
    {
      continue;
    }
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == DoViewChange)))
    {
      int no_recvs = (n / 2) - 1;
      if (!((mbox != NULL) && (mbox->size >= no_recvs)))
      {
        round = StartViewChange_ROUND;
      }
    }
  }
  if (!(pid == get_primary(view_nr, n)) && !(((old_1_msg != NULL) && (old_1_msg->view_nr > view_nr)) && (old_1_msg->label == DoViewChange)) && (((old_3_mbox != NULL) && (old_3_mbox->size >= (n / 2))) && (old_3_mbox->next == NULL)))
  {
    if ((((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == StartViewChange)))
    {
      continue;
    }
    if (!(((msg != NULL) && (msg->view_nr > view_nr)) && (msg->label == DoViewChange)))
    {
      if ((((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
      {
        log->size = mbox->info->log_size;
        log->array = mbox->info->log;
      }
      round = StartViewChange_ROUND;
    }
  }
}


