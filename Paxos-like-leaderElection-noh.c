typedef struct Msg {
 int round;
 int ballot;
 int leader;
} msg;
enum round_typ {NewBallot_ROUND, AckBallot_ROUND, AUX_ROUND} ;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
int timeout();
int reset_timeout();
void out(int v1, int v2);
int all_same(list *mbox, int leader);
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
int coord(int net_size);
int main(int argc, char **argv)
{
    int n = argc;
    int to_all = n+1;
    enum round_typ round;
    int ballot;
    int pid,leader;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    ballot = 0;
    round = NewBallot_ROUND;
    while(1)
    {
        round = NewBallot_ROUND;
        if (pid == coord(n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->ballot = ballot;
            m->round = NewBallot_ROUND;
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        reset_timeout();
        mbox = NULL;
        while(true)
        {
            m = recv();
            if (m != NULL && m->ballot>=ballot && m->round == NewBallot_ROUND){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else { mbox_new->size =1 ;}
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
             leader = mbox->message->leader;
            round = AckBallot_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if (m==0) {abort();}
            m->ballot = ballot;
            m->round = AckBallot_ROUND;
            m->leader = leader;
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
                    if(mbox_new==0) {abort(); }
                    mbox_new->message =m;
                    if(mbox!=0) {mbox_new->size = mbox->size + 1;}
                    else {mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }
                else {free(m);};
                if (timeout()) {
                    break;
                }
                if(mbox != NULL && mbox->size > n/2){break;}
            }
            if (mbox != NULL && mbox->size > n/2){
            if (all_same(mbox,leader)==1){
                out(ballot, leader);
            }
            }
            if(mbox!=0) {
            list_dispose(mbox);
            }
         }else{
             list_dispose1(mbox);
             mbox=NULL;
         }
        ballot++;
        round = NewBallot_ROUND;
    }
    return 1;
}
