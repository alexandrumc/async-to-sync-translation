Launched procedure for nested algorithms


Algorithm B


def round Prepare_ROUND:
  SEND():

  if ((pid == get_primary(view_nr, n)) && ((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))))
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

  if (!(pid == get_primary(view_nr, n)) && (((old_1_mboxB != NULL) && (old_1_mboxB->size == 1)) && (old_1_mboxB->next == NULL)))
  {
    old_0_mboxA = mboxA;
    if (!(((mboxA != NULL) && (mboxA->size == 1)) && (mboxA->next == NULL)))
    {
      bround = DoViewChange_ROUND;
      return_from_inner();
    }
  }
  if ((pid == get_primary(view_nr, n)) && ((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))))
  {
    msgA->message = NULL;
    round = PrepareOk_ROUND;
  }
  if (!(pid == get_primary(view_nr, n)) && (((old_1_mboxB != NULL) && (old_1_mboxB->size == 1)) && (old_1_mboxB->next == NULL)))
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


def round PrepareOk_ROUND:
  SEND():

  if (!(pid == get_primary(view_nr, n)) && (((old_1_mboxB != NULL) && (old_1_mboxB->size == 1)) && (old_1_mboxB->next == NULL)) && (((old_0_mboxA != NULL) && (old_0_mboxA->size == 1)) && (old_0_mboxA->next == NULL)))
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

  UPDATE():

  if ((pid == get_primary(view_nr, n)) && ((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))))
  {
    if (!((mboxA != NULL) && (mboxA->size >= (n / 2))))
    {
      bround = DoViewChange_ROUND;
      return_from_inner();
    }
  }
  if ((pid == get_primary(view_nr, n)) && ((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))))
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
  if (!(pid == get_primary(view_nr, n)) && (((old_1_mboxB != NULL) && (old_1_mboxB->size == 1)) && (old_1_mboxB->next == NULL)) && (((old_0_mboxA != NULL) && (old_0_mboxA->size == 1)) && (old_0_mboxA->next == NULL)))
  {
    
    round = Prepare_ROUND;
  }


Algorithm A


def round DoViewChange_ROUND:
  SEND():

  view_nr++;
  if (!(pid == get_primary(view_nr, n)))
  {
    msgB = malloc(sizeof(msg_ViewChange));
    if ((!msgB))
    {
      abort();
    }
    msgB->view_nr = PHASE;
    msgB->replica_id = pid;
    msgB->label = StartView;
    msgB->log_size = log->size;
    msgB->log = log->array;
    send(msgB, get_primary(PHASE, pid));
  }

  UPDATE():

  if ((pid == get_primary(view_nr, n)))
  {
    old_0_mboxB = mboxB;
    if (((mboxB != NULL) && (mboxB->size >= (n / 2))))
    {
      bround = StartView_ROUND;
    }
  }
  if (!(pid == get_primary(view_nr, n)))
  {
    bround = StartView_ROUND;
  }


def round StartView_ROUND:
  SEND():

  if ((pid == get_primary(view_nr, n)) && ((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))))
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

  UPDATE():

  if ((pid == get_primary(view_nr, n)) && ((old_0_mboxB != NULL) && (old_0_mboxB->size >= (n / 2))))
  {
    inner_algorithm_B(char* client_req, arraylist* log, listB* old_1_mboxB, listB* old_0_mboxB, msg_NormalOp* msgA, commit_list* recovery_buffer);
  }
  bround = DoViewChange_ROUND;
  if (!(pid == get_primary(view_nr, n)))
  {
    if ((((mboxB != NULL) && (mboxB->size == 1)) && (mboxB->next == NULL)))
    {
      inner_algorithm_B(char* client_req, arraylist* log, listB* old_1_mboxB, listB* old_0_mboxB, msg_NormalOp* msgA, commit_list* recovery_buffer);
    }
  }
  bround = DoViewChange_ROUND;


