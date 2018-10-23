/usr/bin/python2.7 /home/dama/PycharmProjects/async-to-sync-translation/ctc-translation/modify.py
No inner algorithm detected

def round CEpoch_ROUND:
  SEND():

if(round == CEpoch_ROUND)
{
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (pid == leader(epoch, n))
        {
          if (!((old_1_mbox != NULL) && (old_1_mbox->size > (n / 2))))
          {
            out();
          }
          m = (msg *) malloc(sizeof(msg));
          if (m == 0)
          {
            abort();
          }
          m->epoch = epoch;
          m->round = NewEpoch_ROUND;
          send(m, to_all);
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (!(pid == leader(epoch, n)))
        {
          m = (msg *) malloc(sizeof(msg));
          if (m == 0)
          {
            abort();
          }
          m->epoch = epoch;
          m->round = CEpoch_ROUND;
          send(m, leader(epoch, n));
        }
      }
    }
  }
}

  UPDATE():

if(round == CEpoch_ROUND)
{
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (pid == leader(epoch, n))
        {
          free(m);
          m = NULL;
          round = Ack_E_ROUND;
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (pid == leader(epoch, n))
        {
          reset_timeout();
          old_1_mbox = mbox;
          if ((mbox != NULL) && (mbox->size > (n / 2)))
          {
            epoch = max_epoch(mbox);
            if (iter != epoch)
            {
              b2 = false;
              continue;
            }
            else
            {
              b2 = true;
            }
            epoch++;
            round = NewEpoch_ROUND;
          }
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (!(pid == leader(epoch, n)))
        {
          free(m);
          m = NULL;
          round = NewEpoch_ROUND;
        }
      }
    }
  }
}


def round NewEpoch_ROUND:
  SEND():

if(round == NewEpoch_ROUND)
{
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!old_0_b2)
      {
        m = (msg *) malloc(sizeof(msg));
        if (m == 0)
        {
          abort();
        }
        m->epoch = epoch;
        m->round = NewEpoch_ROUND;
        send(m, to_all);
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (pid == leader(epoch, n))
        {
          
          m = (msg *) malloc(sizeof(msg));
          if (m == 0)
          {
            abort();
          }
          m->epoch = epoch;
          m->round = NewEpoch_ROUND;
          send(m, to_all);
        }
      }
    }
  }
}

  UPDATE():

if(round == NewEpoch_ROUND)
{
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!old_0_b2)
      {
        free(m);
        m = NULL;
        round = Ack_E_ROUND;
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (pid == leader(epoch, n))
        {
          free(m);
          m = NULL;
          round = Ack_E_ROUND;
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (!(pid == leader(epoch, n)))
        {
          reset_timeout();
          old_0_mbox = mbox;
          if (((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL))
          {
            epoch = mbox->message->epoch;
            if (iter != epoch)
            {
              b1 = false;
              continue;
            }
            else
            {
              b1 = true;
            }
            round = Ack_E_ROUND;
          }
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (!(pid == leader(epoch, n)))
        {
          reset_timeout();
          old_0_mbox = mbox;
          if (!(((mbox != NULL) && (mbox->size == 1)) && (mbox->next == NULL)))
          {
            list_dispose_mbox(mbox);
            epoch++;
            round = CEpoch_ROUND;
          }
        }
      }
    }
  }
}


def round Ack_E_ROUND:
  SEND():

if(round == Ack_E_ROUND)
{
  if (old_0_iter == epoch)
  {
    if (!old_0_b1)
    {
      list_dispose_mbox(mbox);
      m = (msg *) malloc(sizeof(msg));
      if (m == 0)
      {
        abort();
      }
      m->epoch = epoch;
      m->round = Ack_E_ROUND;
      m->history = log;
      m->history_lenght = lastIndex;
      send(m, leader(epoch, n));
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (!(pid == leader(epoch, n)))
        {
          
          list_dispose_mbox(mbox);
          m = (msg *) malloc(sizeof(msg));
          if (m == 0)
          {
            abort();
          }
          m->epoch = epoch;
          m->round = Ack_E_ROUND;
          m->history = log;
          m->history_lenght = lastIndex;
          send(m, leader(epoch, n));
        }
      }
    }
  }
}

  UPDATE():

if(round == Ack_E_ROUND)
{
  if (old_0_iter == epoch)
  {
    if (!old_0_b1)
    {
      free(m);
      m = NULL;
      epoch++;
      round = CEpoch_ROUND;
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!old_0_b2)
      {
        list_dispose_mbox(mbox);
        if (!((mbox != NULL) && (mbox->size > (n / 2))))
        {
          list_dispose_mbox(mbox);
          epoch++;
          round = CEpoch_ROUND;
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (pid == leader(epoch, n))
        {
          list_dispose_mbox(mbox);
          if (!((mbox != NULL) && (mbox->size > (n / 2))))
          {
            list_dispose_mbox(mbox);
            epoch++;
            round = CEpoch_ROUND;
          }
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (!(pid == leader(epoch, n)))
        {
          free(m);
          m = NULL;
          epoch++;
          round = CEpoch_ROUND;
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!old_0_b2)
      {
        list_dispose_mbox(mbox);
        if ((mbox != NULL) && (mbox->size > (n / 2)))
        {
          lastIndex = max_log_size(mbox);
          struct arraylist *old_log = log;
          log = longest_log(mbox, lastIndex);
          list_dispose(old_log);
          epoch++;
          round = CEpoch_ROUND;
        }
      }
    }
  }
  if (old_0_iter == epoch)
  {
    if (!(!old_0_b1))
    {
      if (!(!old_0_b2))
      {
        if (pid == leader(epoch, n))
        {
          list_dispose_mbox(mbox);
          if ((mbox != NULL) && (mbox->size > (n / 2)))
          {
            lastIndex = max_log_size(mbox);
            struct arraylist *old_log = log;
            log = longest_log(mbox, lastIndex);
            list_dispose(old_log);
            epoch++;
            round = CEpoch_ROUND;
          }
        }
      }
    }
  }
}


['CEpoch_ROUND', 'NewEpoch_ROUND', 'Ack_E_ROUND', 'ERR_ROUND']

Process finished with exit code 0
