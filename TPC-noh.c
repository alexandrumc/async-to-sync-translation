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
void send(msg *message, int pid);
int leader(int phase, int net_size);
int all_agree(struct List *l);
int reset_timeout();
enum round_typ {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, FOURTH_ROUND, AUX_ROUND} ;
int main(int argc, char **argv)
{
    int n = argc;
    int to_all = n+1;
    enum round_typ round;
    int phase;
    int pid;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    phase = 0;
    round = AUX_ROUND;
    int response;
    int count = 1;
    int current_command;
    int commit = -1;
    int *log;
    int leader;
    round = AUX_ROUND;
    while (1)
    {
        round = FIRST_ROUND;
        if (pid == leader(phase,n)) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->phase = phase;
            m->round = round;
            m->payload = in();
            m->sender = pid;
            m->response = -1;
            m->commit = -1;
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        reset_timeout();
        mbox = NULL;
        while (1)
        {
            m = recv();
            if (m != NULL && m->phase == phase && m->round == FIRST_ROUND){
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
                {mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else free(m);
            leader = leader(phase,n);
            if (mbox != NULL && mbox->size >= 1) {
                if (mbox->message != NULL && mbox->message->sender == leader){
                    break;
                }
            }
        }
        leader = leader(phase,n);
        if ((mbox->size >= 1 && mbox->message != NULL && mbox->message->sender == leader && mbox->message->payload >0 )) {
            response = rand_bool();
            current_command = mbox->message->payload;
        }
        round = SECOND_ROUND;
        m = (msg *) malloc(sizeof(msg));
        if (m==0) {
            abort();
        }
        m->phase = phase;
        m->round = round;
        m->payload = current_command;
        m->sender = pid;
        m->response = response;
        m->commit = -1;
        send(m, leader(phase,n));
        dispose(m);
        m = NULL;
        if (pid == leader(phase,n)) {
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            while (1)
            {
                m = recv();
                if (m != NULL && m->phase == phase && m->round == SECOND_ROUND){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {
                        abort();
                    }
                    mbox_new->message =m;
                    if(mbox!=0)
                    {
                        mbox_new->size = mbox->size + 1;
                    }
                    else{
                        mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }else free(m);
                if (mbox != NULL && mbox->size == n){
                    break;
                }
            }
            commit = all_agree(mbox);
        }
        round = THIRD_ROUND;
        if (pid == leader(phase,n)) {
            m = (msg *) malloc(sizeof(msg));
            if (m==0) {
                abort();
            }
            m->phase = phase;
            m->round = round;
            m->payload = current_command;
            m->sender = pid;
            m->response = response;
            m->commit = commit;
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        list_dispose(mbox);
        reset_timeout();
        mbox = NULL;
        while (1)
        {
            m = recv();
            if (m != NULL && m->phase == phase && m->round == THIRD_ROUND){
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
                {mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else {free(m);}
            leader = leader(phase,n);
            if (mbox != NULL && mbox->size >= 1) {
                if (mbox->message != NULL && mbox->message->sender == leader){
                    break;
                }
            }
        }
        if (mbox->message != NULL && mbox->message->commit == 1) {
            out(current_command,pid);
        }
        round = FOURTH_ROUND;
        m = (msg *) malloc(sizeof(msg));
        if(m==0) {
            abort();
        }
        m->phase = phase;
        m->round = round;
        m->payload = current_command;
        m->sender = pid;
        m->response = response;
        m->commit = commit;
        send(m, leader(phase,n));
        dispose(m);
        m = NULL;
        if (pid == leader(phase,n)) {
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            while (1)
            {
                m = recv();
                if (m != NULL && m->phase == phase && m->round == FOURTH_ROUND){
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
                        mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }else {free(m);}
                if (mbox!= NULL && mbox->size == n){
                    break;
                }
            }
            list_dispose(mbox);
            mbox = NULL;
            phase = phase + 1;
        }
        else {
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            phase = phase + 1;
        }
        round = AUX_ROUND;
    }
}
