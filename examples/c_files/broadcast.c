typedef struct _msg{
    int op; // operation
    int i; //log entry
    int view;
    int round;
    int lab;
} msg;

int Broadcast (int pid, int leader, int lab, int view, int &lastIndex) {

    msg* mbox;
    int num_mbox = 0;

   //initialization round. To be discussed: it contains all code before
   // the assigment to the first round outside the main loop.
    if(pid == leader){
      if (log[lastIndex][1] == true) {
        lastIndex ++;
        log[lastIndex][0] = in();
        log[lastIndex][0] = true;
      }
    }


    while(true) {
        round = 1; // Prp

        if(pid == leader) {

          m = (msg *) malloc(sizeof(msg));
          m->i = lastIndex;
          m->round = round;
          m->view = view;
          m->lab = lab;
          m->op = log[lastIndex][0];
          m->sender = leader;

          send(m) to all
        }

        //empty mbox
        while(true) {
          m = recv();
          if(m->i==lastIndex && m->view == view && m->round == round && m->lab == lab && m->sender == leader){

          mbox[num_mbox] = *m;
          num_mbox = num_mbox + 1;

          if( num_mbox_propose ==1 ) break;
          if (timeout) break;
          }
        }

        if (timeout) { out()};
        if(num_mbox == 1) {

          log[lastIndex][0] = mbox[num_mbox].op
          log[lastIndex][1] = false;

          round = 2;
          m = (msg *) malloc(sizeof(msg));
          m->i = lastIndex;
          m->round = round;
          m->view = view;
          m->lab = lab;
          m->sender = pid;
          send(m) to leader

          if (pid == leader){
            //empty mbox
            while(true){
              m=recv();
              if (m->i==lastIndex && m->view == view && m->round == round && m->lab == lab ){
                mbox[num_mbox]= *m;
                num_mbox ++;
              }
              if (timeout) break;
              if (num_box> (n/2)) break
            }
            if (num_box> (n/2))
              log[i][1] = true;
              out_external(log[i][0]);

              round = 3;
            else out();
          }
          else{
            round =3;
          }

          if (pid == leader) {
            m = (msg *) malloc(sizeof(msg));
            m->i = i;
            m->round = round;
            m->view = view;
            m->lab = lab;
            m->sender = pid;
            send(m) to all
          }
          //empty mbox
          while(true) {
            m = recv();
            if(m->i==lastIndex && m->view == view && m->round == round && m->lab == lab && m->sender == leader){

            mbox[num_mbox] = *m;
            num_mbox = num_mbox + 1;

            if( num_mbox_propose == 1 ) break;
            if (timeout) break;
            }
          }

          if (timeout) { out()};
          if(num_mbox == 1) {
            log[lastIndex][1] = true;
            log[lastIndex+1][1] = false;
            log[lastIndex+1][0] = in();
            lastIndex++;
            round = 1;
          }
        }
      }
