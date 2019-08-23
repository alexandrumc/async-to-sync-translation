enum round_typ_A {
    CEpoch, NewEpoch, Ack_E, New_Leader, BCAST
};
typedef struct Msg {
    int round;
    int pid;
    int epoch;
    struct arraylist *history;
    int history_lenght;
    int sender;
} msg;
typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;
enum round_typ_B {
    FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND
};
typedef struct MsgB {
    int op;
    int epoch;
    int lab;
    int i;
    int round;
    int sender;
} msgb;
typedef struct Ltype {
    int op;
    int commit;
} ltype;
typedef struct ListB {
    msgb *message;
    struct ListB *next;
    int size;
} listb;
struct arraylist;
struct arraylist *create_arraylist();
ltype *list_get(struct arraylist *a, int i);
int list_length(struct arraylist *a);
void list_add(struct arraylist *a, void *v);
int reset_timeout();
int coord(int net_size);
int timeout();
msg *recv();
void send(msg *message, int pid);
int max_log_size(struct List *mbox);
struct arraylist *longest_log(struct List *mbox, int lastIndex);
void list_dispose_data(struct arraylist *a);
int main(int argc, char **argv)
{
    int n = argc;
    int to_all = n+1;
    struct arraylist *log;
    log = create_arraylist();
    int lastIndex = list_length(log);
    enum round_typ_A round;
    int epoch;
    int pid, leader;
    epoch = 0;
    round = NewEpoch;
    int commit = 0;
    int i;
    enum round_typ_B bround;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    listb *mboxB = NULL;
    listb* mboxB_new = NULL;
    msgb* mB = NULL;
    while (true)
    {
        if(pid == coord(n)){
            round = NewEpoch;
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->epoch = epoch;
            m->round = NewEpoch;
            m->sender = pid;
            send(m, to_all);
            free(m);
            m = NULL;
            round = Ack_E;
            mbox = NULL;
            while(true)
            {
                m = recv();
                if (m != NULL && m->epoch == epoch && m->round == Ack_E){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {abort();}
                    mbox_new->message =m;
                    if(mbox!=0)
                    {mbox_new->size = mbox->size + 1;}
                    else {mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }
                else {free(m);}
                if (timeout()){break;}
                if(mbox != NULL && mbox->size > n/2){
                    break;
                }
            }
            if(mbox != NULL && mbox->size > n/2){
                lastIndex = max_log_size(mbox);
                log = longest_log(mbox, lastIndex);
                round = New_Leader;
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {abort();}
                m->epoch = epoch;
                m->round = New_Leader;
                m->history = log;
                m->history_lenght = lastIndex;
                send(m, to_all);
                free(m);
                m = NULL;
                round = BCAST;
                int len = list_length(log);
                ltype * lastEntry = list_get(log,lastIndex);
                i = lastIndex;
                if (lastEntry!= NULL && lastEntry->commit == 1) {
                    i++;
                    lastIndex++;
                    ltype * newEntry;
                    newEntry = create_ltype(in(),0);
                    list_add(log,newEntry);
                }
                bround = FIRST_ROUND;
                while (true)
                {
                    bround = FIRST_ROUND;
                    mboxB = NULL;
                    mB = (msgb *) malloc(sizeof(msgb));
                    if(mB==0) {
                        abort();
                    }
                    mB->i = i;
                    mB->round = bround;
                    mB->epoch = epoch;
                    mB->lab = BCAST;
                    mB->sender = leader;
                    ltype * entry = list_get(log,lastIndex);
                    if(entry != NULL) {
                        mB->op = entry->op;
                    }
                    send_msgb(mB, to_all);
                    free(mB);
                    mB = NULL;
                    mboxB = NULL;
                    bround = SECOND_ROUND;
                    mB = (msgb *) malloc(sizeof(msgb));
                    if(mB==0) {
                        abort();
                    }
                    mB->i = i;
                    mB->round = bround;
                    mB->epoch = epoch;
                    mB->lab = BCAST;
                    mB->sender = pid;
                    send_msgb(mB, leader);
                    free(mB);
                    mB=NULL;
                    while (true)
                    {
                        mB = recv_msgb();
                        if (mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                            mboxB_new = (listb*) malloc(sizeof(listb));
                            if(mboxB_new==0) {
                                abort();
                            }
                            mboxB_new->message = mB;
                            if(mboxB!=0)
                            {
                                mboxB_new->size = mboxB->size + 1;
                            }
                            else
                            {
                                mboxB_new->size = 1 ;
                            }
                            mboxB_new->next = mboxB;
                            mboxB = mboxB_new;
                        }else {
                            free(mB);
                        }
                        if (timeout())
                            break;
                        if (mboxB != NULL && mboxB->size > n/2)
                            break;
                    }
                    if (mboxB != NULL && mboxB->size > n/2) {
                        ltype *logi = list_get(log,i);
                        if(logi != 0) {
                            logi->commit = 1;
                        }
                        out_external(logi);
                    }
                    else {
                        listB_dispose_no_data(mboxB);
                        mboxB = NULL;
                        break;
                    }
                    bround = THIRD_ROUND;
                    mB = (msgb *) malloc(sizeof(msgb));
                    if(mB==0) {
                        abort();
                    }
                    mB->i = i;
                    mB->round = bround;
                    mB->epoch = epoch;
                    mB->lab = BCAST;
                    mB->sender = pid;
                    send_msgb(mB, to_all);
                    free(mB);
                    mB=NULL;
                    lastIndex++;
                    ltype * newEntry = create_ltype(in(),0);
                    list_add(log,newEntry);
                    bround = FIRST_ROUND;
                    i++;
                    listB_dispose_no_data(mboxB);
                    mboxB = NULL;
                }
                epoch++;
                round = NewEpoch;
                list_dispose_mbox(mbox);
                mbox = NULL;
            }
            else {
                list_dispose_mbox(mbox);
                mbox = NULL;
                epoch++;
                round = NewEpoch;
            }
        }
        else{
            round = NewEpoch;
            mbox= NULL;
            reset_timeout();
            while(true)
            {
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewEpoch){
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
                    }
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                } else {
                    free(m);
                }
                if (timeout()) break;
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    break;
                }
            }
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                epoch = mbox->message->epoch;
                leader = mbox->message->sender;
                round = Ack_E;
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                    abort();
                }
                m->epoch = epoch;
                m->round = Ack_E;
                m->history = log;
                m->history_lenght = lastIndex;
                send(m, leader);
                free(m);
                m = NULL;
                round = New_Leader;
                list_dispose_mbox(mbox);
                mbox = NULL;
                reset_timeout();
                while(true)
                {
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == New_Leader){
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
                            }
                            mbox_new->next = mbox;
                            mbox = mbox_new;
                    } else {
                        free(m);
                    }
                    if (timeout()) break;
                    if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                        break;
                    }
                }
                if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                    lastIndex = mbox->message->history_lenght;
                     log = mbox->message->history;
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    round = BCAST;
                    i = lastIndex;
                    ltype * lastEntry = list_get(log,lastIndex);
                    if (lastEntry!= NULL && lastEntry->commit == 1) {
                        i++;
                        lastIndex++;
                        ltype * newEntry;
                        newEntry = create_ltype(-1,0);
                        list_add(log,newEntry);
                    }
                    bround = FIRST_ROUND;
                    while(true)
                    {
                        bround = FIRST_ROUND;
                        mboxB = NULL;
                        while (true)
                        {
                            mB = recv_msgb();
                            if ( mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                                mboxB_new = (listb*) malloc(sizeof(listb));
                                if(mboxB_new==0) {
                                    abort();
                                }
                                mboxB_new->message =mB;
                                if(mboxB!=0)
                                {
                                    mboxB_new->size = mboxB->size + 1;
                                }
                                else
                                {
                                    mboxB_new->size =1 ;}
                                    mboxB_new->next = mboxB;
                                    mboxB = mboxB_new;
                            }else {
                                free(mB);
                            }
                            if (mboxB != NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader)
                            {
                                break;
                            }
                            if (timeout())
                                break;
                        }
                        if (mboxB!= NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader){
                            ltype *logi = list_get(log,i);
                            if(logi != 0){
                                logi->op = mboxB->message->op;
                                logi->commit = 0;
                            }
                        } else {
                            listB_dispose_no_data(mboxB);
                            mboxB = NULL;
                            break;
                        }
                        bround = SECOND_ROUND;
                        mB = (msgb *) malloc(sizeof(msgb));
                        if(mB==0) {
                            abort();
                        }
                        mB->i = i;
                        mB->round = bround;
                        mB->epoch = epoch;
                        mB->lab = BCAST;
                        mB->sender = pid;
                        send_msgb(mB, leader);
                        free(mB);
                        mB=NULL;
                        bround = THIRD_ROUND;
                        listB_dispose_no_data(mboxB);
                        mboxB = NULL;
                        while (true)
                        {
                            mB = recv_msgb();
                            if (mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                                mboxB_new = (listb*) malloc(sizeof(listb));
                                if(mboxB_new==0) {
                                    abort();
                                }
                                mboxB_new->message =mB;
                                if(mboxB!=0)
                                {
                                    mboxB_new->size = mboxB->size + 1;
                                }
                                else
                                {
                                    mboxB_new->size =1 ;}
                                    mboxB_new->next = mboxB;
                                    mboxB = mboxB_new;
                            }else {
                                free(mB);
                            }
                            if (mboxB != NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader)
                            {
                                break;
                            }
                            if (timeout())
                                break;
                        }
                        if (mboxB != NULL && mboxB->size >= 1) {
                            ltype *logi = list_get(log,i);
                            if(logi != 0){
                                logi->commit = 1;
                                out(logi);
                                lastIndex++;
                                ltype * newEntry = create_ltype(-1,0);
                                list_add(log,newEntry);
                            }
                            listB_dispose_no_data(mboxB);
                            mboxB = NULL;
                        }else{
                            listB_dispose_no_data(mboxB);
                            mboxB = NULL;
                            break;
                        }
                        bround = FIRST_ROUND;
                        i++;
                    }
                    epoch++;
                    round = NewEpoch;
                }
                else {
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    epoch++;
                    round = NewEpoch;
                }
            }
            else {
                list_dispose_mbox(mbox);
                mbox = NULL;
                epoch++;
                round = NewEpoch;
            }
        }
    }
    return 1;
}
void list_dispose_mbox(struct List *l);
void send_msgb(msgb* message, int pid);
msgb* recv_msgb();
void listB_dispose_no_data(struct ListB *l);
void out(ltype *v);
void out_external(ltype *v);
int in();
ltype * create_ltype(int op, int b);
