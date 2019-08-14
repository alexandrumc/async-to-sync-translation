No inner algorithm detected

def round StartViewChange_ROUND:
  SEND():

  m = (msg *) malloc(sizeof(msg));
  send((void *) m, to_all);
  if (((((m->view == view) && (m->round == StartView_ROUND)) && (round == StartView_ROUND)) && (follower(n, view) == pid)))
  {
    view++;
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, to_all);
  }

  UPDATE():

  
  if (((((m->view == view) && (m->round == StartView_ROUND)) && (round == StartView_ROUND)) && (follower(n, view) == pid)))
  {
    continue;
  }
  


def round DoViewChange_ROUND:
  SEND():

  if ((((((m->view == view) && (m->round == StartViewChange_ROUND)) && (round == StartViewChange_ROUND)) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, primary(n, PHASE));
  }

  UPDATE():

  
  if (((((m->view == view) && (m->round == DoViewChange_ROUND)) && (round == StartViewChange_ROUND)) && (primary(n, view) == pid)))
  {
    continue;
  }
  


def round StartView_ROUND:
  SEND():

  if ((((((m->view == view) && (m->round == DoViewChange_ROUND)) && (round == DoViewChange_ROUND)) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    m = (msg *) malloc(sizeof(msg));
    send((void *) m, to_all);
  }

  UPDATE():

  if ((((((m->view == view) && (m->round == StartViewChange_ROUND)) && (round == StartViewChange_ROUND)) && (follower(n, view) == pid)) && collected_all_start_view_change(mbox, view)))
  {
    continue;
  }
  if (((((m->view == view) && (m->round == StartView_ROUND)) && (round == StartView_ROUND)) && (follower(n, view) == pid)))
  {
    out(PHASE);
  }
  if ((((((m->view == view) && (m->round == DoViewChange_ROUND)) && (round == DoViewChange_ROUND)) && (primary(n, view) == pid)) && collected_all_do_view_change(mbox, view)))
  {
    out(PHASE);
  }


