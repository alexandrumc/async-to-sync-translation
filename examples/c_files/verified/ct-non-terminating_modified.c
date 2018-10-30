
typedef struct Msg {
    int round;
    int phase;
    int estimate;
    int timestamp;
    int ack;
    int sender;
} msg;




typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;


void dispose(msg* c);


void list_dispose(struct List *l);

int in();

int leader(int phase, int net_size);


msg* max_timestamp(struct List* mbox);







msg* recv();


void send(msg* message, int pid);


void out(int pid, int est);


int timeout();


enum round_typ {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, FOURTH_ROUND, AUX_ROUND} ;

int main(int argc, char**argv)
{
    int state = 0;
    
    int round = 0;
    int old_round = round;
    int myid = 0;
    int timestamp = 0;
    
    int phase = 0;
    int old_phase = -1;
    
    int estimate = in();
    int leader = 0;
    
    int n = argc;
    int to_all = n+1;
    
    
    
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    
    
    int retry;
    int timeout;
    int ack = 0;
    
    msg* m = NULL;
    
    volatile int random;
    
    round = FIRST_ROUND;
    
    
    while(true)
    {
        old_round = round;
        round = FIRST_ROUND;
        
        
        old_phase = phase;
        old_round = round;
        
        m = NULL;
        m = (msg*)malloc(sizeof(msg));
        if (m == 0) { abort(); }
        
        m->phase = phase;
        m->round = round;
        m->estimate = estimate;
        m->sender = myid;
        m->timestamp = timestamp;
        
        leader = leader(phase,n);
        send(m,leader);
        
        dispose(m);
        m = NULL;
        mbox = NULL;
        
        if (myid == leader) {
            if (mbox!= 0) { list_dispose(mbox); }
            mbox = NULL;
            
            while (1)
            {
                m = recv();
                if (m->round == 1 && m->phase == phase && m->round == round) {
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) { abort(); }
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else  mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }else{free(m);}
                
                timeout =timeout();
                if (timeout==0) break;
                if (mbox!=0 && mbox->size >= (n + 1) / 2)
                    break;
            }
            
            if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                
                m = max_timestamp(mbox);
                estimate = m->estimate;
                free(m);
                m=NULL;
            }else{
                
                list_dispose(mbox);
                mbox = NULL;
            }
        }
        
        
        if (mbox!= 0) { list_dispose(mbox); }
        mbox = NULL;
        
        
        old_round = round;
        round = SECOND_ROUND;
        
        if (myid == leader) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) { abort(); }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            send(m,to_all);
            dispose(m);
            m = NULL;
        }
        
        while (1)
        {
            m = recv();
            if (m->phase == phase && m->round == SECOND_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) { abort(); }
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else{free(m);}
            
            timeout =timeout();
            if (timeout==0) break;
            if (mbox!=0 && mbox->message->sender ==leader )
                break;
            
        }
        
        if (mbox!=0 && mbox->message->sender ==leader ) {
            estimate = mbox->message->estimate;
            timestamp = phase;
            
        }else{
            
            list_dispose(mbox);
            mbox = NULL;
        }
        
        old_round = round;
        round = THIRD_ROUND;
        
        if (timestamp == phase) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) { abort(); }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            send(m,leader);
            dispose(m);
            m = NULL;
        }
        
        if (mbox!= 0) { list_dispose(mbox); }
        mbox = NULL;
        if (myid == leader) {
            while (1)
                
            {
                
                m = recv();
                if (m->phase == phase && m->round == round) {
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) { abort(); }
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else  mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }else{free(m);}
                
                
                timeout =timeout();
                if (timeout==0) break;
                
                if (mbox!=0 && mbox->size >= (n + 1) / 2)
                    break;
                
                
            }
            
            if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                
                ack = 1;
                
            }else{
                
                list_dispose(mbox);
                mbox = NULL;
            }
        }
        
        old_round = round;
        round = FOURTH_ROUND;
        
        if (myid == leader && ack == 1) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) { abort(); }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->ack = ack;
            send(m,to_all);
            dispose(m);
            m = NULL;
        }
        
        
        
        
        if (mbox!= 0) { list_dispose(mbox); }
        
        mbox = NULL;
        
        while (1)
        {
            
            m = recv();
            if (m->round == FOURTH_ROUND && m->phase == phase) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) { abort(); }
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else{free(m);}
            
            timeout =timeout();
            if (timeout==0) break;
            
            if (mbox!=0 )break;
            
        }
        
        
        
        if (mbox!=0)
        {
            estimate = mbox->message->estimate;
            state = 1;
            out(myid,estimate);
            
        }
        
        
        if (mbox!= 0) { list_dispose(mbox); }
        mbox = NULL;
        
        old_phase = phase;
        
        phase = phase + 1;
        round = AUX_ROUND;
    }
    return 1;
}

