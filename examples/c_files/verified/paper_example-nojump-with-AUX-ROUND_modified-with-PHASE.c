
typedef struct Msg {
	int round;
	int phase;
} msg;

typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;




int timeout();

int reset_timeout();



enum round_typ {FIRST_ROUND, SECOND_ROUND, AUX_ROUND} ;

msg* recv();

void dispose(msg* c);


void list_dispose(struct List *l);


void dispose_list(list* c);

void send(msg* message, int pid);

int leader(int phase, int net_size);

int main(int argc, char **argv)
{
    
    int n = argc;
    
    int to_all = n+1;
    
    int* log = (int*)malloc(100*sizeof(int));
    if(log==0){ abort();}
    
    enum round_typ round;
    int phase;
    int pid;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    phase = 0;
    


    round = AUX_ROUND;
    enum round_typ old_round = round;
    while(true)
    {
    
        round = FIRST_ROUND;
        old_round = round;
        if (pid == leader(phase,n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {abort();}
            m->phase = phase;
            m->round = FIRST_ROUND;
            
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        reset_timeout();
        mbox = NULL;
        while(true)
        {
            m = recv();
            if (m != NULL && m->phase == phase && m->round == FIRST_ROUND){
                
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0){ abort();}
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
               
            }
            else free(m);
            int timeout = timeout();
            if (timeout==0 || (mbox != NULL && mbox->size ==1)){
                break;
            }
        }
        if(mbox!=NULL){
        
           
            round = SECOND_ROUND;

            m = (msg *) malloc(sizeof(msg));
            if (m==0) {abort();}
            m->phase = phase;
            m->round = SECOND_ROUND;
            send(m, to_all);
            
            dispose(m);
            m = NULL;
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            while(true)
            {
                 m = recv();
                 if (m != NULL && m->phase == phase && m->round == SECOND_ROUND){
            
                                mbox_new = (list*) malloc(sizeof(list));
                                if(mbox_new==0) {abort();}
                                mbox_new->message =m;
                                if(mbox!=0)
                                    mbox_new->size = mbox->size + 1;
                                else  mbox_new->size =1 ;
                                mbox_new->next = mbox;
                                mbox = mbox_new;
            
                    }
                    else free(m);
                   int timeout = timeout();
                   if (timeout==0 || (mbox != NULL && mbox->size > n/2)){
                                break;
                    }

            }
            
            
            if (mbox != NULL && mbox->size > n/2){
                    printf("\n%d", phase);
                    int leader = leader(phase,n);
             }
        	
           
            if(mbox!=0) {list_dispose(mbox);}
        }
       
        phase++;
        round = AUX_ROUND;
        
    }
    return 1;
}


