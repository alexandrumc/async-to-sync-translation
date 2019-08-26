enum round_typ {NewBallot_ROUND, AckBallot_ROUND, AUX_ROUND} ;
typedef struct Msg {
 int round;
 int ballot;
 int leader;
} msg;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
struct arraylist;
struct arraylist *create_arraylist() ;
struct arraylist *create_arraylist2() ;
int list_length(struct arraylist *a);
void list_add(struct arraylist *a, void *v);
void list_remove_nth(struct arraylist *a, int n);
int all_same(list *mbox, int leader);
void list_dispose1(struct List *l);
void out(int v1, int v2);
int main(int argc, char **argv)
{
    int n = argc;
    int to_all = n+1;
    struct arraylist *log;
    log = create_arraylist();
    int lastIndex = list_length(log);
    enum round_typ round;
    int ballot;
    int pid,leader;
    ballot = 0;
    round = NewBallot_ROUND;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    round = NewBallot_ROUND;
    while (true)
    {
        if(pid == coord(n)){
            round = NewBallot_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {abort();}
            m->ballot = ballot;
            m->round = NewBallot_ROUND;
            m->leader = pid;
            send(m, to_all);
            free(m);
            m = NULL;
            mbox= NULL;
            reset_timeout();
            while(true)
            {
                m = recv();
                if (m != NULL && m->ballot >= ballot && m->round == NewBallot_ROUND){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {abort();}
                    mbox_new->message =m;
                    if(mbox!=0){mbox_new->size = mbox->size + 1;}
                    else{mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                } else {
                    free(m);}
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    break;
                }
                if (timeout()){
                    break;}
            }
   if(iter == ballot && mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                leader = mbox->message->leader;
                round = AckBallot_ROUND;
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {abort();}
                m->ballot = ballot;
                m->round = AckBallot_ROUND;
                m->leader = leader;
                send(m, to_all);
                free(m);
                m = NULL;
                list_dispose1(mbox);
                mbox = NULL;
                while(true)
                {
                    m = recv();
                    if (m != NULL && m->ballot == ballot && m->round == AckBallot_ROUND){
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) {abort();}
                        mbox_new->message =m;
                        if(mbox!=0)
                            {mbox_new->size = mbox->size + 1; }
                        else{mbox_new->size =1 ;}
                        mbox_new->next = mbox;
                        mbox = mbox_new;
                    }
                    else {free(m);}
                    if (timeout()){
                        break;
                    }
                    if(mbox != NULL && mbox->size > n/2){
                        break;
                    }
                }
                if(mbox != NULL && mbox->size > n/2){
                     if (all_same(mbox,leader)==1){
                   out(ballot, leader);
                }
                }
                if(mbox!=0) {list_dispose(mbox);}
                    ballot++;
                    round = NewBallot_ROUND;
            } else {
                list_dispose1(mbox);
                mbox = NULL;
                ballot++;
                round = NewBallot_ROUND;
            }
        }
        else{
            round = NewBallot_ROUND;
            mbox= NULL;
            reset_timeout();
            while(true)
            {
                m = recv();
                if (m != NULL && m->ballot >= ballot && m->round == NewBallot_ROUND){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {
                    abort();
                    }
                    mbox_new->message =m;
                    if(mbox!=0)
                        {mbox_new->size = mbox->size + 1; }
                    else
                    {mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                } else {
                    free(m);}
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    break;
                }
                if (timeout()){
                    break;}
            }
   if(iter == ballot && mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                leader = mbox->message->leader;
                round = AckBallot_ROUND;
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                abort();
                }
                m->ballot = ballot;
                m->round = AckBallot_ROUND;
                m->leader = leader;
                send(m, to_all);
                free(m);
                m = NULL;
                list_dispose1(mbox);
                mbox = NULL;
                while(true)
                {
                    m = recv();
                    if (m != NULL && m->ballot == ballot && m->round == AckBallot_ROUND){
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) {
                        abort();
                        }
                        mbox_new->message =m;
                        if(mbox!=0)
                            {
                            mbox_new->size = mbox->size + 1;
                            }
                        else { mbox_new->size =1 ;}
                        mbox_new->next = mbox;
                        mbox = mbox_new;
                    }
                    else {free(m);}
                    if (timeout()){
                        break;
                    }
                    if(mbox != NULL && mbox->size > n/2){
                        break;
                    }
                }
                if(mbox != NULL && mbox->size > n/2){
                    if (all_same(mbox,leader)==1){
                    out(ballot, leader);
                }
                }
                if(mbox!=0) {list_dispose(mbox);}
                    ballot++;
                round = NewBallot_ROUND;
        }else {
                list_dispose1(mbox);
                mbox = NULL;
                ballot++;
                round = NewBallot_ROUND;
            }
 }
    }
    return 1;
}
int reset_timeout();
int coord(int net_size);
int timeout();
msg* recv();
void send(msg* message, int pid);
int max_log_size(struct List* mbox);
struct arraylist* longest_log(struct List* mbox, int lastIndex);
void list_dispose(struct List *l);
