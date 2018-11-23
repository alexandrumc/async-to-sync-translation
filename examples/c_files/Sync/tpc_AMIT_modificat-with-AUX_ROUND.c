def round FIRST_ROUND:
  SEND():

if(round == FIRST_ROUND){
  if ((pid == leader(phase, n)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->phase = phase;
    m->payload = in();
    m->sender = pid;
    m->response = -1;
    m->commit = -1;
    send(m, to_all);
  }
}

  UPDATE():

if(round == FIRST_ROUND)
{



  old_0_mbox = mbox;
  if (((mbox->message != NULL) && (mbox->message->sender == leader)))
  {
    leader = leader(phase, n);
    if (((((mbox->size >= 1) && (mbox->message != NULL)) && (mbox->message->sender == leader)) && (mbox->message->payload > 0)))
    {
      response = rand_bool();
      current_command = mbox->message->payload;
    }
  }
}


def round SECOND_ROUND:
  SEND():

if(round == SECOND_ROUND){
  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->phase = phase;
    m->payload = current_command;
    m->sender = pid;
    m->response = response;
    m->commit = -1;
    send(m, leader(phase, n));
  }
}

  UPDATE():

if(round == SECOND_ROUND)
{
  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)))
  {

  }

  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)))
  {
    if ((pid == leader(phase, n)))
    {
      if (((mbox != NULL) && (mbox->size == n)))
      {
        commit = all_agree(mbox);
      }
    }
  }

}


def round THIRD_ROUND:
  SEND():

if(round == THIRD_ROUND){
  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)))
  {
    if ((pid == leader(phase, n)))
    {
      m = (msg *) malloc(sizeof(msg));
      if ((m == 0))
      {
        abort();
      }
      m->phase = phase;
      m->payload = current_command;
      m->sender = pid;
      m->response = response;
      m->commit = commit;
      send(m, to_all);
    }
  }
}

  UPDATE():

if(round == THIRD_ROUND)
{
  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)))
  {


  }
  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)))
  {

    old_1_mbox = mbox;
    if (((mbox->message != NULL) && (mbox->message->sender == leader)))
    {
      if (((mbox->message != NULL) && (mbox->message->commit == 1)))
      {
        out(current_command, pid);
      }
    }
  }
}


def round FOURTH_ROUND:
  SEND():

if(round == FOURTH_ROUND){
  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)) && ((old_1_mbox->message != NULL) && (old_1_mbox->message->sender == leader)))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->phase = phase;
    m->payload = current_command;
    m->sender = pid;
    m->response = response;
    m->commit = commit;
    send(m, leader(phase, n));
  }
}

  UPDATE():

if(round == FOURTH_ROUND)
{
  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)) && ((old_1_mbox->message != NULL) && (old_1_mbox->message->sender == leader)))
  {

  }

  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)) && ((old_1_mbox->message != NULL) && (old_1_mbox->message->sender == leader)))
  {
    if ((pid == leader(phase, n)))
    {
      if (((mbox != NULL) && (mbox->size == n)))
      {
        phase = phase + 1;
      }
    }
  }

  if (((old_0_mbox->message != NULL) && (old_0_mbox->message->sender == leader)) && ((old_1_mbox->message != NULL) && (old_1_mbox->message->sender == leader)))
  {
    if (!(pid == leader(phase, n)))
    {
      phase = phase + 1;
    }
  }
}
