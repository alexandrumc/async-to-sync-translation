struct arraylist;
struct arraylist *create_arraylist() ;
struct arraylist *create_arraylist2() ;
int list_length(struct arraylist *a);
void list_add(struct arraylist *a, void *v);
void list_remove_nth(struct arraylist *a, int n);
void list_dispose(struct arraylist* a);
typedef struct Msg {
    int round;
    int pid;
    int epoch;
    struct arraylist *history;
    int history_lenght;
    int sender;
} msg;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
void list_dispose1(struct List *l);
int main(int argc, char **argv)
{
    int n = argc;
    int to_all = n+1;
    struct arraylist *log;
    log = create_arraylist();
    int lastIndex = list_length(log);
    enum round_typ_A round;
    int epoch;
    int pid;
    epoch = 0;
    round = NewBallot_ROUND;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    round = AUX_ROUND;
    while (true)
    {
        if(pid == leader(epoch,n)){
            round = NewBallot_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->epoch = epoch;
            m->round = NewBallot_ROUND;
            send(m, to_all);
            free(m);
            m = NULL;
            mbox= NULL;
            reset_timeout();
            while(true)
            {
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewBallot_ROUND){
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
                    free(m);}
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    break;
                }
                if (timeout()){
                    break;}
            }
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                epoch = mbox->message->epoch;
                round = AckBallot_ROUND;
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                abort();
                }
                m->epoch = epoch;
                m->round = AckBallot_ROUND;
                m->history = log;
                m->history_lenght = lastIndex;
                send(m, leader(epoch,n));
                free(m);
                m = NULL;
                list_dispose1(mbox);
                mbox = NULL;
                while(true)
                {
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == AckBallot_ROUND){
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
                    else {free(m);}
                    if (timeout()){
                        break;
                    }
                    if(mbox != NULL && mbox->size > n/2){
                        break;
                    }
                }
                if(mbox != NULL && mbox->size > n/2){
                    lastIndex = max_log_size(mbox);
                    struct arraylist* old_log = log;
                    log = longest_log(mbox, lastIndex);
                    list_dispose(old_log);
                    epoch++;
                    round = AUX_ROUND;
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                }else{
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    epoch++;
                    round = AUX_ROUND;
                    break;
                }
            } else {
                list_dispose1(mbox);
                mbox = NULL;
                epoch++;
                round = AUX_ROUND;
                break;
            }
        }
        else{
            round = NewBallot_ROUND;
            mbox= NULL;
            reset_timeout();
            while(true)
            {
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewBallot_ROUND){
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
                    free(m);}
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    break;
                }
                if (timeout()){
                    break;}
            }
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                epoch = mbox->message->epoch;
                round = AckBallot_ROUND;
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                abort();
                }
                m->epoch = epoch;
                m->round = AckBallot_ROUND;
                m->history = log;
                m->history_lenght = lastIndex;
                send(m, leader(epoch,n));
                free(m);
                m = NULL;
                list_dispose1(mbox);
                mbox = NULL;
                while(true)
                {
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == AckBallot_ROUND){
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
                    else {free(m);}
                    if (timeout()){
                        break;
                    }
                    if(mbox != NULL && mbox->size > n/2){
                        break;
                    }
                }
                if(mbox != NULL && mbox->size > n/2){
                    lastIndex = max_log_size(mbox);
                    struct arraylist* old_log = log;
                    log = longest_log(mbox, lastIndex);
                    list_dispose(old_log);
                    epoch++;
                    round = AUX_ROUND;
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                }else{
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    epoch++;
                    round = NewBallot_ROUND;
                    break;
                }
            } else {
                list_dispose1(mbox);
                mbox = NULL;
                epoch++;
                round = AUX_ROUND;
            }
        }
    }
    return 1;
}
int reset_timeout();
int leader(int phase, int net_size);
int timeout();
msg* recv();
void send(msg* message, int pid);
int max_log_size(struct List* mbox);
struct arraylist* longest_log(struct List* mbox, int lastIndex);
void list_dispose_mbox(struct List *l);
