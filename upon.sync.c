No inner algorithm detected

def round StartViewChange_ROUND:
  SEND():

if(round == StartViewChange_ROUND){
  m = (msg *) malloc(sizeof(msg));
  m->round = StartViewChange_ROUND;
  send((void *) m, to_all);
  if (((((old_m->view == view) && (old_m->round == StartView_ROUND)) && (old_round == StartView_ROUND)) && (follower(n, view) == old_pid)))
  {
    view++;
    m = (msg *) malloc(sizeof(msg));
    m->round = StartViewChange_ROUND;
    send((void *) m, to_all);
  }
}

  UPDATE():

if(round == StartViewChange_ROUND)
{
  if ((((((m->view == view) && (m->round == StartViewChange_ROUND)) && (round == StartViewChange_ROUND)) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    old_mbox = mbox;
    old_round = round;
    old_m = m;
    old_pid = pid;
    round = DoViewChange_ROUND;
  }
  if (((((old_m->view == view) && (old_m->round == StartView_ROUND)) && (old_round == StartView_ROUND)) && (follower(n, view) == old_pid)))
  {
    continue;
  }
  if (((((m->view == view) && (m->round == DoViewChange_ROUND)) && (round == StartViewChange_ROUND)) && (primary(n, view) == pid)))
  {
    old_mbox = mbox;
    old_round = round;
    old_m = m;
    old_pid = pid;
    round = DoViewChange_ROUND;
  }
}


def round DoViewChange_ROUND:
  SEND():

if(round == DoViewChange_ROUND){
  if ((((((old_m->view == view) && (old_m->round == StartViewChange_ROUND)) && (old_round == StartViewChange_ROUND)) && (follower(n, view) == old_pid)) && collected_all_start_view_change(old_mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    m->round = DoViewChange_ROUND;
    send((void *) m, primary(n, PHASE));
  }
}

  UPDATE():

if(round == DoViewChange_ROUND)
{
  if ((((((old_m->view == view) && (old_m->round == StartViewChange_ROUND)) && (old_round == StartViewChange_ROUND)) && (follower(n, view) == old_pid)) && collected_all_start_view_change(old_mbox, view)))
  {
    old_mbox = mbox;
    old_round = round;
    old_m = m;
    old_pid = pid;
    round = StartView_ROUND;
  }
  if (((((old_m->view == view) && (old_m->round == DoViewChange_ROUND)) && (old_round == StartViewChange_ROUND)) && (primary(n, view) == old_pid)))
  {
    continue;
  }
  if ((((((m->view == view) && (m->round == DoViewChange_ROUND)) && (round == DoViewChange_ROUND)) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    old_mbox = mbox;
    old_round = round;
    old_m = m;
    old_pid = pid;
    round = StartView_ROUND;
  }
}


def round StartView_ROUND:
  SEND():

if(round == StartView_ROUND){
  if ((((((old_m->view == view) && (old_m->round == DoViewChange_ROUND)) && (old_round == DoViewChange_ROUND)) && (primary(n, view) == old_pid)) && collected_all_do_view_change(old_mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    m->round = StartView_ROUND;
    send((void *) m, to_all);
  }
}

  UPDATE():

if(round == StartView_ROUND)
{
  if ((((((old_m->view == view) && (old_m->round == StartViewChange_ROUND)) && (old_round == StartViewChange_ROUND)) && (follower(n, view) == old_pid)) && collected_all_start_view_change(old_mbox, view)))
  {
    continue;
  }
  if (((((m->view == view) && (m->round == StartView_ROUND)) && (round == StartView_ROUND)) && (follower(n, view) == pid)))
  {
    out(PHASE);
  }
  if ((((((old_m->view == view) && (old_m->round == DoViewChange_ROUND)) && (old_round == DoViewChange_ROUND)) && (primary(n, view) == old_pid)) && collected_all_do_view_change(old_mbox, view)))
  {
    out(PHASE);
  }
}
