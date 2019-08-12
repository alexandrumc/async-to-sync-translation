{
  round = StartViewChange_ROUND;
  if (round == StartViewChange_ROUND)
  {
    m = (msg *) malloc(sizeof(msg));
    m->view = view;
    m->round = StartViewChange_ROUND;
    send((void *) m, to_all);
  }

  if (((round == StartViewChange_ROUND) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view))
  {
    round = DoViewChange_ROUND;
    m = (msg *) malloc(sizeof(msg));
    m->view = view;
    m->round = DoViewChange_ROUND;
    send((void *) m, primary(n, view));
    round = StartView_ROUND;
    continue;
  }

  if (((round == StartViewChange_ROUND) && (primary(n, view) == pid)) && collected_all_start_view_change(mbox, view))
  {
    round = DoViewChange_ROUND;
    continue;
  }

  if (((round == DoViewChange_ROUND) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))
  {
    round = StartView_ROUND;
    m = (msg *) malloc(sizeof(msg));
    m->view = view;
    m->round = StartView_ROUND;
    send((void *) m, to_all);
    out(view);
    round = StartViewChange_ROUND;
    view++;
    continue;
  }

  if (((round == StartView_ROUND) && (follower(n, view) == pid)) && (mbox->size >= 1))
  {
    out(view);
    round = AUX_ROUND;
    view++;
    continue;
  }

}

No inner algorithm detected

def round StartViewChange_ROUND:
  SEND():

if(round == StartViewChange_ROUND){
  if ((round == StartViewChange_ROUND))
  {
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, to_all);
  }
}

  UPDATE():

if(round == StartViewChange_ROUND)
{
  if ((((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    continue;
  }
  if ((((jump == True) && (follower(n, view) == pid)) && (mbox->size >= 1)))
  {
    out(PHASE);
  }
  
  
  
  old_0_round = ROUND;
  if (!(((round == StartViewChange_ROUND) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    old_2_round = ROUND;
    if (!(((round == StartViewChange_ROUND) && (primary(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
    {
      old_1_round = ROUND;
      if (!(jump == True) && (((jump == True) && (follower(n, view) == pid)) && (mbox->size >= 1)))
      {
        out(PHASE);
      }
    }
  }
  
  old_0_round = ROUND;
  if (!(((round == StartViewChange_ROUND) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    old_2_round = ROUND;
    if (!(((round == StartViewChange_ROUND) && (primary(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
    {
      
    }
  }
  
  old_0_round = ROUND;
  if (!(((round == StartViewChange_ROUND) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    
  }
}


def round DoViewChange_ROUND:
  SEND():

if(round == DoViewChange_ROUND){
  if ((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, primary(n, PHASE));
  }
}

  UPDATE():

if(round == DoViewChange_ROUND)
{
  
  if ((((jump == True) && (primary(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    continue;
  }
  
  if ((((jump == True) && (primary(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    continue;
  }
  old_1_round = ROUND;
  if (!(((round == DoViewChange_ROUND) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)) && (((jump == True) && (follower(n, view) == pid)) && (mbox->size >= 1)))
  {
    out(PHASE);
  }
}


def round StartView_ROUND:
  SEND():

if(round == StartView_ROUND){
  if ((((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, to_all);
  }
}

  UPDATE():

if(round == StartView_ROUND)
{
  if ((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    continue;
  }
  
  if ((((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    out(PHASE);
  }
  if ((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    continue;
  }
  if (!(jump == True) && !(jump == True) && (((round == StartView_ROUND) && (follower(n, view) == pid)) && (mbox->size >= 1)))
  {
    out(PHASE);
  }
}


