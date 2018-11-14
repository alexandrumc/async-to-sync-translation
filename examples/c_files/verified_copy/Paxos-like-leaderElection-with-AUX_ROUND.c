typedef struct Msg {
 int round;
 int ballot;
 int sender;
} msg;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
int timeout();
int reset_timeout();
void out(int v1, int v2);
void send(msg* message, int pid);
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
void list_dispose(struct List *l);
void list_dispose1(struct List *l);
void dispose_list(list* c);
int leader(int phase, int net_size)
{
    int res = phase % net_size;
    return res;
}
int main(int argc, char **argv)
{
    int n = argc;
    int to_all = n+1;
    enum round_typ round;
    int ballot;
    int pid;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    ballot = 0;
    round = AUX_ROUND;
    while(1)
    {
        round = NEW_BALLOT_ROUND;
        if (pid == leader(ballot,n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->ballot = ballot;
            m->round = NEW_BALLOT_ROUND;
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        reset_timeout();
        mbox = NULL;
        while(true)
        {
            m = recv();
            if (m != NULL && m->ballot>=ballot && m->round == NEW_BALLOT_ROUND){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0){
                    mbox_new->size = mbox->size + 1;
                    }
                else { mbox_new->size =1 ;
                 }
                mbox_new->next = mbox;
                mbox = mbox_new;
            }
            else {
             free(m);
             }
            if(mbox != NULL && mbox->size ==1 && mbox->next == NULL){
             break;
            }
            if (timeout()){
                break;}
        }
         if(mbox!=NULL && mbox->size ==1 && mbox->next == NULL){
             ballot = mbox->message->ballot;
            round = ACK_BALLOT_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if (m==0) {
            abort();
            }
            m->ballot = ballot;
            m->round = ACK_BALLOT_ROUND;
            send(m, to_all);
            dispose(m);
            m = NULL;
            list_dispose1(mbox);
            reset_timeout();
            mbox = NULL;
            while(true)
            {
                 m = recv();
                 if (m != NULL && m->ballot == ballot && m->round == round){
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
                    }
                    else free(m);
                    if (timeout()) break;
                    if(mbox != NULL && mbox->size > n/2){
                                break;
                    }
            }
            if (mbox != NULL && mbox->size > n/2){
                printf("\n%d", ballot);
                int leader = leader(ballot,n);
                out(ballot, leader);
            }
            if(mbox!=0) {
            list_dispose(mbox);
            }
        }
        ballot++;
        round = AUX_ROUND;
    }
    return 1;
}
