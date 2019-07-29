Launched procedure for nested algorithms


Algorithm B


def round Prepare_ROUND:
  SEND():

  if ((pid == get_primary(view_nr, n)) && (((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)) && ((old_1_mboxB != NULL) && (old_1_mboxB->size >= (n / 2))))
  {
    recovery_buffer = NULL;
    msgA = malloc(sizeof(msg_NormalOp));
    if ((!msgA))
    {
      abort();
    }
    client_req = in();
    if ((!client_req))
    {
      msgA->message = prepare_ping();
    }
    else
    {
      msgA->message = malloc(strlen(client_req) * (sizeof(char)));
      if ((!msgA->message))
      {
        abort();
      }
      strcpy(msgA->message, client_req);
    }
    msgA->commiting = 0;
    if (((log->size > 0) && (log->array[log->size - 1]->commited == 1)))
    {
      msgA->commiting = 1;
      msgA->op_number = log->array[log->size - 1]->op_number;
    }
    msgA->view_nr = view_nr;
    msgA->request_nr = PHASE;
    msgA->replica_id = -1;
    msgA->label = Prepare;
    if ((client_req))
    {
      add_entry_log(create_log_entry(view_nr, PHASE, client_req), log);
      op_number++;
    }
    send((void *) msgA, to_all);
  }

  UPDATE():

  if ((pid == get_primary(view_nr, n)) && (old_0_transfer_to_algo == 1))
  {
    old_0_mboxA = mboxA;
    if (!(((mboxA != NULL) && (mboxA->size == 1)) && (mboxA->next == NULL)))
    {
      bround = StartViewChange_ROUND;
      return_from_inner();
    }
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mboxB != NULL) && (old_2_mboxB->size >= (n / 2))) && (old_2_mboxB->next == NULL)) && (((old_3_mboxB != NULL) && (old_3_mboxB->size == 1)) && (old_3_mboxB->next == NULL)))
  {
    old_2_mboxA = mboxA;
    if (!(((mboxA != NULL) && (mboxA->size == 1)) && (mboxA->next == NULL)))
    {
      bround = StartViewChange_ROUND;
      return_from_inner();
    }
  }
  if ((pid == get_primary(view_nr, n)) && (((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)) && ((old_1_mboxB != NULL) && (old_1_mboxB->size >= (n / 2))))
  {
    msgA->message = NULL;
    round = PrepareOk_ROUND;
  }
  if ((pid == get_primary(view_nr, n)) && (old_0_transfer_to_algo == 1))
  {
    old_0_mboxA = mboxA;
    if ((((mboxA != NULL) && (mboxA->size == 1)) && (mboxA->next == NULL)))
    {
      old_1_mboxA = mboxA;
      if ((((mboxA != NULL) && (mboxA->size == 1)) && (mboxA->next == NULL)))
      {
        round = PrepareOk_ROUND;
      }
    }
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mboxB != NULL) && (old_2_mboxB->size >= (n / 2))) && (old_2_mboxB->next == NULL)) && (((old_3_mboxB != NULL) && (old_3_mboxB->size == 1)) && (old_3_mboxB->next == NULL)))
  {
    old_2_mboxA = mboxA;
    if ((((mboxA != NULL) && (mboxA->size == 1)) && (mboxA->next == NULL)))
    {
      old_3_mboxA = mboxA;
      if ((((mboxA != NULL) && (mboxA->size == 1)) && (mboxA->next == NULL)))
      {
        round = PrepareOk_ROUND;
      }
    }
  }


def round PrepareOk_ROUND:
  SEND():

  if ((pid == get_primary(view_nr, n)) && (old_0_transfer_to_algo == 1) && (((old_0_mboxA != NULL) && (old_0_mboxA->size == 1)) && (old_0_mboxA->next == NULL)))
  {
    if ((((old_1_mboxA != NULL) && (old_1_mboxA->size == 1)) && (old_1_mboxA->next == NULL)))
    {
      msgA = malloc(sizeof(msg_NormalOp));
      msgA->label = PrepareOk;
      msgA->view_nr = view_nr;
      msgA->replica_id = pid;
      msgA->request_nr = mboxA->info->request_nr;
      send(msgA, get_primary(view_nr, n));
    }
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mboxB != NULL) && (old_2_mboxB->size >= (n / 2))) && (old_2_mboxB->next == NULL)) && (((old_3_mboxB != NULL) && (old_3_mboxB->size == 1)) && (old_3_mboxB->next == NULL)) && (((old_2_mboxA != NULL) && (old_2_mboxA->size == 1)) && (old_2_mboxA->next == NULL)))
  {
    if ((((old_3_mboxA != NULL) && (old_3_mboxA->size == 1)) && (old_3_mboxA->next == NULL)))
    {
      msgA = malloc(sizeof(msg_NormalOp));
      msgA->label = PrepareOk;
      msgA->view_nr = view_nr;
      msgA->replica_id = pid;
      msgA->request_nr = mboxA->info->request_nr;
      send(msgA, get_primary(view_nr, n));
    }
  }

  UPDATE():

  if ((pid == get_primary(view_nr, n)) && (((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)) && ((old_1_mboxB != NULL) && (old_1_mboxB->size >= (n / 2))))
  {
    if (!((mboxA != NULL) && (mboxA->size >= (n / 2))))
    {
      bround = StartViewChange_ROUND;
      return_from_inner();
    }
  }
  if ((pid == get_primary(view_nr, n)) && (((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)) && ((old_1_mboxB != NULL) && (old_1_mboxB->size >= (n / 2))))
  {
    if (((mboxA != NULL) && (mboxA->size >= (n / 2))))
    {
      if (((mboxA != NULL) && (mboxA->size >= (n / 2))))
      {
        log->array[log->size - 1]->commited = 1;
      }
      round = Prepare_ROUND;
    }
  }
  if ((pid == get_primary(view_nr, n)) && (old_0_transfer_to_algo == 1) && (((old_0_mboxA != NULL) && (old_0_mboxA->size == 1)) && (old_0_mboxA->next == NULL)))
  {
    
    round = Prepare_ROUND;
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mboxB != NULL) && (old_2_mboxB->size >= (n / 2))) && (old_2_mboxB->next == NULL)) && (((old_3_mboxB != NULL) && (old_3_mboxB->size == 1)) && (old_3_mboxB->next == NULL)) && (((old_2_mboxA != NULL) && (old_2_mboxA->size == 1)) && (old_2_mboxA->next == NULL)))
  {
    
    round = Prepare_ROUND;
  }


Algorithm A


def round StartViewChange_ROUND:
  SEND():

if(round == StartViewChange_ROUND){
  view_nr++;
  transfer_to_algo = -1;
  if ((pid == get_primary(view_nr, n)))
  {
    msgB = malloc(sizeof(msg_ViewChange *));
    if ((!msgB))
    {
      abort();
    }
    msgB->view_nr = PHASE;
    msgB->replica_id = pid;
    msgB->label = StartViewChange;
    send((void *) msgB, to_all);
  }
  view_nr++;
  transfer_to_algo = -1;
  if (!(pid == get_primary(view_nr, n)))
  {
    msgB = malloc(sizeof(msg_ViewChange *));
    if ((!msgB))
    {
      abort();
    }
    msgB->view_nr = PHASE;
    msgB->replica_id = pid;
    msgB->label = StartViewChange;
    send((void *) msgB, to_all);
  }
}

  UPDATE():

if(round == StartViewChange_ROUND)
{
  if ((pid == get_primary(view_nr, n)))
  {
    old_0_mboxB = mboxB;
    if ((((mboxB != NULL) && (mboxB->size >= (n / 2))) && (mboxB->next == NULL)))
    {
      transfer_to_algo = -1;
      bround = DoViewChange_ROUND;
    }
  }
  if (!(pid == get_primary(view_nr, n)))
  {
    old_2_mboxB = mboxB;
    if ((((mboxB != NULL) && (mboxB->size >= (n / 2))) && (mboxB->next == NULL)))
    {
      transfer_to_algo = -1;
      bround = DoViewChange_ROUND;
    }
  }
}


def round DoViewChange_ROUND:
  SEND():

if(round == DoViewChange_ROUND){
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mboxB != NULL) && (old_2_mboxB->size >= (n / 2))) && (old_2_mboxB->next == NULL)))
  {
    msgB = malloc(sizeof(msg_ViewChange));
    if ((!msgB))
    {
      abort();
    }
    msgB->view_nr = PHASE;
    msgB->replica_id = pid;
    msgB->label = DoViewChange;
    msgB->log_size = log->size;
    msgB->log = log->array;
    msgB->prev_view = prev_view;
    send(msgB, get_primary(PHASE, pid));
  }
}

  UPDATE():

if(round == DoViewChange_ROUND)
{
  if ((pid == get_primary(view_nr, n)) && (((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)))
  {
    old_1_mboxB = mboxB;
    if (((mboxB != NULL) && (mboxB->size >= (n / 2))))
    {
      bround = StartView_ROUND;
    }
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_2_mboxB != NULL) && (old_2_mboxB->size >= (n / 2))) && (old_2_mboxB->next == NULL)))
  {
    bround = StartView_ROUND;
  }
  if ((pid == get_primary(view_nr, n)) && (((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)))
  {
    old_1_mboxB = mboxB;
    if (!((mboxB != NULL) && (mboxB->size >= (n / 2))))
    {
      if ((transfer_to_algo == 1))
      {
        status = normal;
        op_number = 0;
        inner_algorithm_B(arraylist* log, listB* old_3_mboxB, msg_NormalOp* msgA, commit_list* recovery_buffer, listB* old_1_mboxB, listB* old_0_mboxB, char* client_req, listB* old_2_mboxB);
      }
      bround = StartViewChange_ROUND;
    }
  }
}


def round StartView_ROUND:
  SEND():

if(round == StartView_ROUND){
  if ((pid == get_primary(view_nr, n)))
  {
    if ((((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)) && ((old_1_mboxB != NULL) && (old_1_mboxB->size >= (n / 2))))
    {
      msgB = malloc(sizeof(msg_ViewChange));
      if ((!msgB))
      {
        abort();
      }
      msgB->view_nr = PHASE;
      msgB->label = StartView;
      msgB->log_size = log->size;
      msgB->log = log->array;
      send((void *) msgB, to_all);
    }
  }
}

  UPDATE():

if(round == StartView_ROUND)
{
  if ((pid == get_primary(view_nr, n)))
  {
    if ((((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))) && (old_0_mboxB->next == NULL)) && ((old_1_mboxB != NULL) && (old_1_mboxB->size >= (n / 2))))
    {
      status = normal;
      op_number = 0;
      inner_algorithm_B(arraylist* log, listB* old_3_mboxB, msg_NormalOp* msgA, commit_list* recovery_buffer, listB* old_1_mboxB, listB* old_0_mboxB, char* client_req, listB* old_2_mboxB);
    }
    if ((transfer_to_algo == 1))
    {
      status = normal;
      op_number = 0;
      inner_algorithm_B(arraylist* log, listB* old_3_mboxB, msg_NormalOp* msgA, commit_list* recovery_buffer, listB* old_1_mboxB, listB* old_0_mboxB, char* client_req, listB* old_2_mboxB);
    }
    bround = StartViewChange_ROUND;
  }
  if (!(pid == get_primary(view_nr, n)))
  {
    if ((((old_2_mboxB != NULL) && (old_2_mboxB->size >= (n / 2))) && (old_2_mboxB->next == NULL)))
    {
      if ((((mboxB != NULL) && (mboxB->size == 1)) && (mboxB->next == NULL)))
      {
        status = normal;
        op_number = 0;
        inner_algorithm_B(arraylist* log, listB* old_3_mboxB, msg_NormalOp* msgA, commit_list* recovery_buffer, listB* old_1_mboxB, listB* old_0_mboxB, char* client_req, listB* old_2_mboxB);
      }
    }
    bround = StartViewChange_ROUND;
  }
}



