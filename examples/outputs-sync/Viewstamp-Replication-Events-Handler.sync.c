No inner algorithm detected

def round StartViewChange_ROUND:
  SEND():

  UPDATE():

if(round == StartViewChange_ROUND)
{
  if (((((((old_0_round == StartViewChange_ROUND) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((jump == True) && (follower(n, view) == pid)) && (jump == True))) || (((old_0_round == StartViewChange_ROUND) && (primary(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))))
  {
    if ((((jump == True) && (follower(n, view) == pid)) && (jump == True)))
    {
      continue;
    }
    old_3_round = ROUND;
  }
  if (((((((old_0_round == StartViewChange_ROUND) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((jump == True) && (follower(n, view) == pid)) && (jump == True))) || (((old_0_round == StartViewChange_ROUND) && (primary(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))))
  {
    if ((((jump == True) && (follower(n, view) == pid)) && (jump == True)))
    {
      continue;
    }
    old_3_round = ROUND;

  }
  if (((((((old_0_round == StartViewChange_ROUND) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((jump == True) && (follower(n, view) == pid)) && (jump == True))) || (((old_0_round == StartViewChange_ROUND) && (primary(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))))
  {
    if ((((jump == True) && (follower(n, view) == pid)) && (jump == True)))
    {
      continue;
    }
    old_3_round = ROUND;
    old_1_m = m;
    if (!(((round == StartViewChange_ROUND) && (primary(n, view) == pid)) && (m->round == DoViewChange_ROUND)))
    {

    }
  }
}


def round DoViewChange_ROUND:
  SEND():

if(round == DoViewChange_ROUND){
  if (((((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((jump == True) && (follower(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && (old_0_m->round == DoViewChange_ROUND))) || (((old_0_round == DoViewChange_ROUND) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))) && (((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, primary(n, PHASE));
  }
}

  UPDATE():

if(round == DoViewChange_ROUND)
{

  if (((((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((jump == True) && (follower(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && (old_0_m->round == DoViewChange_ROUND))) || (((old_0_round == DoViewChange_ROUND) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))))
  {
    if ((((jump == True) && (primary(n, view) == pid)) && (old_1_m->round == DoViewChange_ROUND)))
    {
      continue;
    }

  }
}


def round StartView_ROUND:
  SEND():

if(round == StartView_ROUND){
  if (((((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((old_0_round == StartView_ROUND) && (follower(n, view) == pid)) && (old_0_m->round == StartView_ROUND))) || (((jump == True) && (primary(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))) && (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, to_all);
  }
}

  UPDATE():

if(round == StartView_ROUND)
{
  if (((((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((old_0_round == StartView_ROUND) && (follower(n, view) == pid)) && (old_0_m->round == StartView_ROUND))) || (((jump == True) && (primary(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))))
  {
    if ((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
    {
      continue;
    }
    if ((((round == StartView_ROUND) && (follower(n, view) == pid)) && (m->round == StartView_ROUND)))
    {
      out(PHASE);
    }
  }
  if (((((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((old_0_round == StartView_ROUND) && (follower(n, view) == pid)) && (old_0_m->round == StartView_ROUND))) || (((jump == True) && (primary(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))) && (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    out(PHASE);
  }
  if (((((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)) || (((old_0_round == StartView_ROUND) && (follower(n, view) == pid)) && (old_0_m->round == StartView_ROUND))) || (((jump == True) && (primary(n, view) == pid)) && (jump == True))) || (((jump == True) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view))))
  {
    if ((((jump == True) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
    {
      continue;
    }

  }
}