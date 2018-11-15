typedef struct Msg {
    int round;
    int phase;
    int payload;
    int response;
    int sender;
    int commit;
} msg;
typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;
void out(int v1, int v2);
int in();
int rand_bool();
msg *recv();
void dispose(msg *c);
void list_dispose(struct List *l);
void list_dispose_no_data(struct List *l);
void dispose_list(list *c);
void send(msg *message, int pid);
int leader(int phase, int net_size);
int all_agree(struct List *l);
int main(int argc, char**argv)
{
    int state = 0;
    int round = 0;
    int myid = 0;
    int timestamp = 0;
    int phase = 0;
    int estimate = in();
    int leader = 0;
    int n = argc;
    int to_all = n+1;
    list *mbox = NULL;
    list* mbox_new = NULL;
    int ack = 0;
    msg* m = NULL;
    round = FIRST_ROUND;
    phase = 1;
    ack = rand_bool();
    while(1)
    {
        round = FIRST_ROUND;
        leader = leader(phase,n);
        m = NULL;
        m = (msg *) malloc(sizeof(msg));
        if (m == 0) {
        abort();
        }
        m->phase = phase;
        m->round = round;
        m->estimate = estimate;
        m->sender = myid;
        m->timestamp = timestamp;
        m->ack = 0;
        send(m, leader);
        dispose(m);
        m = NULL;
        mbox = NULL;
        if (myid == leader) {
            while (true)
                {
                    m = recv();
                    if (m!= NULL && m->round == FIRST_ROUND && m->phase == phase) {
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) {
                        abort();
                        }
                        mbox_new->message =m;
                        if(mbox!=0)
                        {
                            mbox_new->size = mbox->size + 1;
                            }
                        else
                        {
                        mbox_new->size =1 ;
                        mbox_new->next = mbox;
                        mbox = mbox_new;
                        }
                        if (mbox->size >= (n + 1) / 2) {
                                break;
                            }
                        }else if (m!=NULL && m->round == FOURTH_ROUND ){
                            mbox_new = (list*) malloc(sizeof(list));
                            if(mbox_new==0) {
                            abort();
                            }
                            mbox_new->message = m;
                            if(mbox!=0)
                                {
                                mbox_new->size = mbox->size + 1;
                                }
                            else
                            {
                            mbox_new->size =1 ;
                            mbox_new->next = mbox;
                            mbox = mbox_new;
                            }
                            break;
                        } else free(m);
                    }
            if(mbox!=0 && mbox->message!=NULL){
             if (mbox->message->round == FOURTH_ROUND){
                    round = FOURTH_ROUND;
                    estimate = mbox->message->estimate;
                    state = 1;
                    list_dispose(mbox);
                    mbox = NULL;
                    break;
             }else{
                  if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                     m = max_timestamp(mbox);
                     estimate = m->estimate;
                     free(m);
                     m=NULL;
                 }
             }
            }
            list_dispose(mbox);
            mbox = NULL;
    }
        round = SECOND_ROUND;
        if (myid == leader) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            send(m,to_all);
            dispose(m);
            m = NULL;
        }
        list_dispose(mbox);
        mbox = NULL;
        while (1)
        {
            m = recv();
            if (m!= NULL && m->phase == phase && m->round == SECOND_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0)
                    {
                    mbox_new->size = mbox->size + 1;
                    }
                else
                {
                mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
                }
            } else {
                if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0)
                    {
                    mbox_new->size = mbox->size + 1;
                    }
                else
                {
                mbox_new->size = 1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
                }
                break;
                } else free(m);
            }
            if (timeout()) break;
            else if (mbox!=0 && mbox->message->sender ==leader){
                break;
                }
        }
        if(mbox!=0 && mbox->message!=NULL){
            if( mbox->message->round == FOURTH_ROUND){
             round = FOURTH_ROUND;
             estimate = mbox->message->estimate;
             state = 1;
                  list_dispose(mbox);
                  mbox = NULL;
             break;
            }else{
                if (mbox!=0) {
                    estimate = mbox->message->estimate;
                    timestamp = phase;
                }
            }
        }
        list_dispose(mbox);
        mbox = NULL;
        round = THIRD_ROUND;
        if (timestamp == phase) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            send(m,leader);
            dispose(m);
            m = NULL;
        }
        if (myid == leader) {
            mbox = NULL;
            while (1)
            {
                m = recv();
                if (m!= NULL && m->round == THIRD_ROUND && m->phase == phase) {
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {
                    abort();
                    }
                    mbox_new->message =m;
                    if(mbox!=0)
                        {
                        mbox_new->size = mbox->size + 1;
                        }
                    else
                    {
                    mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                    }
                    if (mbox->size > n/2)
                    {
                     break;
                    }
                }else if (m!= NULL && m->round == FOURTH_ROUND) {
                            mbox_new = (list*) malloc(sizeof(list));
                            if(mbox_new==0) {
                            abort();
                            }
                            mbox_new->message =m;
                            if(mbox!=0)
                                {
                                mbox_new->size = mbox->size + 1;
                                }
                            else
                            {
                            mbox_new->size =1 ;
                            mbox_new->next = mbox;
                            mbox = mbox_new;
                            }
                            break;
                        }else {free(m);}
                if (timeout()) break;
            }
            if(mbox!=0 && mbox->message!=NULL){
             if (mbox->message->round == FOURTH_ROUND){
                    round = FOURTH_ROUND;
                    estimate = mbox->message->estimate;
                    state = 1;
                    list_dispose(mbox);
                    mbox = NULL;
                    break;
             }else{
                 if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                         ack =1;
                  }
                 }
             }
             list_dispose(mbox);
             mbox = NULL;
        }
        round = FOURTH_ROUND;
        if (myid == leader && ack == 1) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->ack = ack;
            send(m,to_all);
            dispose(m);
            m = NULL;
        }
        list_dispose(mbox);
        mbox = NULL;
        while (1)
        {
            m = recv();
            if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0)
                    {
                    mbox_new->size = mbox->size + 1;
                    }
                else
                {
                mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
                break;
                }
            }else{free(m);}
            if (timeout()) break;
        }
        if (mbox!=0)
        {
            estimate = mbox->message->estimate;
            state = 1;
            out(myid,estimate);
            list_dispose(mbox);
            mbox = NULL;
            break;
        }
        phase = phase + 1;
        round = FIRST_ROUND;
    }
    return 1;
}
