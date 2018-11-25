No inner algorithm detected

def round FIRST_ROUND:
  SEND():

if(round == FIRST_ROUND){
  leader = leader(PHASE, n);
  m = (msg *) malloc(sizeof(msg));
  if ((m == 0))
  {
    abort();
  }
  m->round = round;
  m->estimate = estimate;
  m->sender = myid;
  m->timestamp = timestamp;
  m->ack = 0;
  send(m, leader);
}

  UPDATE():

if(round == FIRST_ROUND)
{
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if (!((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
    {
      round = ERR_ROUND;
    }
  }
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if (((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
    {
      old_1_mbox = mbox;
      if ((((mbox != 0) && (mbox->message != NULL)) && (jump == True)) && (jump == True))
      {
        round = FOURTH_ROUND;
      }
    }
  }
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if (((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
    {
      old_1_mbox = mbox;
      if ((((mbox != 0) && (mbox->message != NULL)) && (jump == True)))
      {
        if (!(jump == True))
        {
          if (((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
          {
            m = max_timestamp(mbox);
            estimate = m->estimate;
          }
          round = SECOND_ROUND;
        }
      }
    }
  }
  if ((myid == leader))
  {
    old_0_mbox = mbox;
    if (((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
    {
      old_1_mbox = mbox;
      if (!(jump == True))
      {
        round = SECOND_ROUND;
      }
    }
  }
  if (!(myid == leader))
  {
    round = SECOND_ROUND;
  }
}


def round SECOND_ROUND:
  SEND():

if(round == SECOND_ROUND){
  if ((myid == leader))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->sender = myid;
    m->round = round;
    m->estimate = estimate;
    m->timestamp = timestamp;
    send(m, to_all);
  }
}

  UPDATE():

if(round == SECOND_ROUND)
{
  
  old_2_mbox = mbox;
  if (!(((mbox != NULL) && (mbox->message != NULL)) && ((mbox->message->sender == leader) || (jump == True))))
  {
    round = ERR_ROUND;
  }
  
  old_2_mbox = mbox;
  if ((((mbox != NULL) && (mbox->message != NULL)) && ((mbox->message->sender == leader) || (jump == True))))
  {
    old_3_mbox = mbox;
    if (((mbox != 0) && (mbox->message != NULL)) && (jump == True))
    {
      round = FOURTH_ROUND;
    }
  }
  
  old_2_mbox = mbox;
  if ((((mbox != NULL) && (mbox->message != NULL)) && ((mbox->message->sender == leader) || (jump == True))))
  {
    old_3_mbox = mbox;
    if (((mbox != 0) && (mbox->message != NULL)))
    {
      if (!(jump == True))
      {
        estimate = mbox->message->estimate;
        timestamp = PHASE;
        ack = 1;
        round = THIRD_ROUND;
      }
    }
  }
  
  old_2_mbox = mbox;
  if ((((mbox != NULL) && (mbox->message != NULL)) && ((mbox->message->sender == leader) || (jump == True))))
  {
    old_3_mbox = mbox;
    if (!((mbox != 0) && (mbox->message != NULL)))
    {
      ack = -1;
      round = THIRD_ROUND;
    }
  }
}


def round THIRD_ROUND:
  SEND():

if(round == THIRD_ROUND){
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (jump == True))))
  {
    m = (msg *) malloc(sizeof(msg));
    if ((m == 0))
    {
      abort();
    }
    m->sender = myid;
    m->round = round;
    m->estimate = estimate;
    m->timestamp = timestamp;
    m->ack = ack;
    send(m, leader);
  }
}

  UPDATE():

if(round == THIRD_ROUND)
{
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (jump == True))))
  {
    if ((myid == leader))
    {
      old_4_mbox = mbox;
    }
  }
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (jump == True))))
  {
    if ((myid == leader))
    {
      old_4_mbox = mbox;
      if (!((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
      {
        round = ERR_ROUND;
      }
    }
  }
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (jump == True))))
  {
    if ((myid == leader))
    {
      old_4_mbox = mbox;
      if (((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
      {
        old_5_mbox = mbox;
        if (((mbox != 0) && (mbox->message != NULL)) && (jump == True))
        {
          round = FOURTH_ROUND;
        }
      }
    }
  }
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (jump == True))))
  {
    if ((myid == leader))
    {
      old_4_mbox = mbox;
      if (((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
      {
        old_5_mbox = mbox;
        if (((mbox != 0) && (mbox->message != NULL)))
        {
          if (!(jump == True))
          {
            if (((mbox != 0) && (mbox->size >= ((n + 1) / 2))))
            {
              if ((all_ack(mbox)))
              {
                ack = 1;
              }
              else
              {
                ack = -1;
              }
            }
            round = FOURTH_ROUND;
          }
        }
      }
    }
  }
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (jump == True))))
  {
    if ((myid == leader))
    {
      old_4_mbox = mbox;
      if (((((mbox != NULL) && (mbox->message != NULL)) && (jump == True)) || ((mbox != NULL) && (mbox->size >= ((n + 1) / 2)))))
      {
        old_5_mbox = mbox;
        if (!((mbox != 0) && (mbox->message != NULL)))
        {
          round = FOURTH_ROUND;
        }
      }
    }
  }
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (jump == True))))
  {
    if (!(myid == leader))
    {
      round = FOURTH_ROUND;
    }
  }
}


def round FOURTH_ROUND:
  SEND():

if(round == FOURTH_ROUND){
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (old_2_mbox->message->round == FOURTH_ROUND))))
  {
    if (((myid == leader) && (ack == 1)))
    {
      m = (msg *) malloc(sizeof(msg));
      if ((m == 0))
      {
        abort();
      }
      m->sender = myid;
      m->round = round;
      m->estimate = estimate;
      m->ack = ack;
      send(m, to_all);
    }
  }
}

  UPDATE():

if(round == FOURTH_ROUND)
{
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (old_2_mbox->message->round == FOURTH_ROUND))))
  {
    if (((old_3_mbox != 0) && (old_3_mbox->message != NULL)) && (mbox->message->round == FOURTH_ROUND))
    {
      estimate = mbox->message->estimate;
      state = 1;
      out(myid, estimate);
    }
    round = THIRD_ROUND;
  }
  if ((myid == leader) && ((((old_0_mbox != NULL) && (old_0_mbox->message != NULL)) && (old_0_mbox->message->round == FOURTH_ROUND)) || ((old_0_mbox != NULL) && (old_0_mbox->size >= ((n + 1) / 2)))) && (((old_1_mbox != 0) && (old_1_mbox->message != NULL)) && (old_1_mbox->message->round == FOURTH_ROUND)) && (mbox->message->round == FOURTH_ROUND))
  {
    estimate = mbox->message->estimate;
    state = 1;
    out();
  }
  round = SECOND_ROUND;
  if ((((old_2_mbox != NULL) && (old_2_mbox->message != NULL)) && ((old_2_mbox->message->sender == leader) || (old_2_mbox->message->round == FOURTH_ROUND))))
  {
    
    if ((mbox != 0))
    {
      estimate = mbox->message->estimate;
      state = 1;
      out(myid, estimate);
    }
    round = FIRST_ROUND;
  }
}


