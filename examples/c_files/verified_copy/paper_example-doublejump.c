typedef struct Msg {
    int round;
    int epoch;
} msg;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
int max_epoch(struct List* mbox)
{
    struct List *current = mbox;
    int max = mbox->message->epoch;
    current = mbox->next;
    while (current != NULL)
    {
        if(current->message->epoch > max)
            max = current->message->epoch;
        current = current->next;
    }
    return max;
}
int timeout();
msg* recv()
{
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) {
    abort();
    }
    return m;
}
void dispose(msg* c)
{
    free(c);
}
void list_dispose(struct List *l)
{
    struct List* current = l;
    while(current != 0)
    {
        struct List* oldcurrent = current;
        msg *m = current->message;
        current = current->next;
        free(m);
        free(oldcurrent);
    }
}
void dispose_list(list* c)
{
    free(c);
}
int leader(int phase, int net_size)
{
    int res = phase % net_size;
    return res;
}
int reset_timeout();
void out(int v1, int v2);
void send(msg* message, int pid);
int main(int argc, char **argv)
{
    int n = argc;
    enum round_typ round;
    int epoch, old_epoch;
    int pid;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    epoch = 0;
    old_epoch = epoch-1;
    int to_all = n+1;
    round = FIRST_ROUND;
    while(1)
    {
        round = FIRST_ROUND;
        old_epoch = epoch;
        old_round = round;
        if (pid == leader(epoch,n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->epoch = epoch;
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
            if (m != NULL && m->epoch >= epoch && m->round == FIRST_ROUND){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0)
                    {
                    mbox_new->size = mbox->size + 1;
                    }
                else {
                mbox_new->size =1 ;

                mbox_new->next = mbox;
                mbox = mbox_new;
                }
            }
            else free(m);
            if (timeout()) break;
            else if(mbox != NULL && mbox->size ==1){
                break;
            }
        }
        if(mbox!=NULL){
            old_epoch = epoch;
            epoch = mbox->message->epoch;
            round = mbox->message->round;
            struct List* next = mbox->next;
            old_epoch = epoch;
            old_round = round;
            round = SECOND_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if (m==0) {
            abort();
            }
            m->epoch = epoch;
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
                if (m != NULL && m->epoch >= epoch && m->round == SECOND_ROUND){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {
                    abort();
                    }
                    mbox_new->message =m;
                    if(mbox!=0)
                        {
                        mbox_new->size = mbox->size + 1;
                        }
                    else {
                    mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                    }
                }
                else free(m);
                if (timeout()) break;
                else if(mbox != NULL && mbox->size > n/2){
                    break;
                }
            }
            if (mbox != NULL && mbox->size > n/2){
                epoch = max_epoch(mbox);
                printf("\n%d", epoch);
                int leader = leader(epoch,n);
                out(epoch, leader);
                epoch++;
                old_round = round;
                round = FIRST_ROUND;
            }
            else{
                old_epoch = epoch;
                epoch++;
                old_round = round;
                round = FIRST_ROUND;
            }
            if(mbox!=0) {
            list_dispose(mbox);
            }
        }else{
            old_epoch = epoch;
            epoch++;
            old_round = round;
            round = FIRST_ROUND;
        }
    }
    return 1;
}
