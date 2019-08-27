typedef struct Msg {
    int round;
    int phase;
    int estimate;
    int timestamp;
    int ack;
    int sender;
} msg;
typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;
enum round_typ {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, FOURTH_ROUND, AUX_ROUND} ;
void out(int v1, int v2);
int in();
int timeout();
int reset_timeout();
int rand_bool();
msg *recv();
msg* max_timestamp(struct List* mbox);
int all_ack(struct List* mbox);
void dispose(msg *c);
void list_dispose(struct List *l);
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
    if (m == 0) {abort();}
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
                if(mbox_new==0) {abort();}
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else{ mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else if (m!=NULL && m->round == FOURTH_ROUND ){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort();}
                mbox_new->message = m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else{ mbox_new->size =1 ; }
                mbox_new->next = mbox;
                mbox = mbox_new;
                break;
            } else {free(m);};
            if (mbox!=NULL && mbox->message != NULL &&mbox->message->round ==FOURTH_ROUND){break;}
            if(mbox!=NULL && mbox->size >= (n + 1) / 2) {
                break;
            }
        }
        if(mbox!=0 && mbox->message!=NULL) {
         if(mbox->message->round == FOURTH_ROUND){
                round = FOURTH_ROUND;
                estimate = mbox->message->estimate;
                state = 1;
                out(myid, estimate);
                mbox = NULL;
                break;
                round = AUX_ROUND;
            }else {
               if (mbox->size >= (n + 1) / 2) {
                    m = max_timestamp(mbox);
                    estimate = m->estimate;
                    free(m);
                    m=NULL;
                }
         }
        mbox = NULL;
     }
    }
    round = SECOND_ROUND;
    if (myid == leader) {
        m = (msg *) malloc(sizeof(msg));
        if(m==0) {abort();}
        m->sender = myid;
        m->phase = phase;
        m->round = round;
        m->estimate = estimate;
        m->timestamp = timestamp;
        send(m,to_all);
        dispose(m);
        m = NULL;
    }
    mbox = NULL;
    while (1)
    {
        m = recv();
        if (m!= NULL && m->phase == phase && m->round == SECOND_ROUND) {
            mbox_new = (list*) malloc(sizeof(list));
            if(mbox_new==0) {abort();}
            mbox_new->message =m;
            if(mbox!=0){mbox_new->size = mbox->size + 1; }
            else{mbox_new->size =1 ;}
            mbox_new->next = mbox;
            mbox = mbox_new;
        } else {
            if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort();}
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1; }
                else {mbox_new->size = 1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            } else {free(m);}
        }
        if(timeout()) {break; }
        if (mbox!=NULL && mbox->message != NULL && (mbox->message->sender ==leader || mbox->message->round ==FOURTH_ROUND )){
            break;
        }
    }
    if(mbox!=0 && mbox->message!=NULL){
        if( mbox->message->round == FOURTH_ROUND){
            round = FOURTH_ROUND;
            estimate = mbox->message->estimate;
            state = 1;
            out(myid, estimate);
            mbox = NULL;
            break;
        } else{
            estimate = mbox->message->estimate;
            timestamp = phase;
            ack = 1;
        }
    }
    else {ack = -1;}
    mbox = NULL;
    round = THIRD_ROUND;
    m = (msg *) malloc(sizeof(msg));
    if(m==0) {abort();}
    m->sender = myid;
    m->phase = phase;
    m->round = round;
    m->estimate = estimate;
    m->timestamp = timestamp;
    m->ack = ack;
    send(m,leader);
    dispose(m);
    m = NULL;
    mbox = NULL;
    if (myid == leader) {
        while (1)
        {
            m = recv();
            if (m!= NULL && m->round == round && m->phase == phase) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort();}
                mbox_new->message =m;
                if(mbox!=0) {mbox_new->size = mbox->size + 1; }
                else { mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort(); }
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else{ mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else {free(m);}
            if (mbox!=NULL && mbox->message != NULL && mbox->message->round ==FOURTH_ROUND){break;}
            if(mbox!=NULL && mbox->size >= (n + 1) / 2) {
                break;
                round = AUX_ROUND;
            }
        }
        if(mbox!=0 && mbox->message!=NULL &&mbox->message->round == FOURTH_ROUND){
                round = FOURTH_ROUND;
                estimate = mbox->message->estimate;
                state = 1;
                out(myid, estimate);
                mbox = NULL;
                break;
                round = AUX_ROUND;
        }else{
            if(all_ack(mbox)) { ack =1;}
            else {ack = -1;}
        }
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
    mbox = NULL;
    while (1)
    {
        m = recv();
        if (m!= NULL && m->round == FOURTH_ROUND) {
            mbox_new = (list*) malloc(sizeof(list));
            if(mbox_new==0) {abort();}
            mbox_new->message =m;
            if(mbox!=0){mbox_new->size = mbox->size + 1;}
            else{mbox_new->size =1 ;}
            mbox_new->next = mbox;
            mbox = mbox_new;
            break;
        }else{free(m);}
        if (timeout()){
            break; }
    }
    if (mbox!=0)
    {
        estimate = mbox->message->estimate;
        state = 1;
        out(myid,estimate);
        mbox = NULL;
        break;
        round = AUX_ROUND;
    }
    phase = phase + 1;
    round = FIRST_ROUND;
}
return 1;
}
