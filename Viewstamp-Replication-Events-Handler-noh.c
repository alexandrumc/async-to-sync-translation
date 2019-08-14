typedef struct Msg {
 int round;
 int view;
} msg;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
enum round_typ {StartViewChange_ROUND, DoViewChange_ROUND, StartView_ROUND, AUX_ROUND} ;
int primary(int n, int view);
void out(int v1);
int timeout();
void send(msg* message, int pid);
msg* recv();
void dispose(msg* c);
int all_same(list* mbox, int view);
void list_dispose(list* l);
int main(int argc, char **argv)
{
    int pid;
    int to_all = n+1;
    enum round_typ round;
    int view;
    list* mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    view = 0;
    round = StartViewChange_ROUND;
    m = (msg *) malloc(sizeof(msg));
    m->view = view;
    m->round = StartViewChange_ROUND;
    send((void*)m, to_all);
    while(true)
    {
        while(true)
        {
            m = recv();
            if (m != NULL){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message = m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else { mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else{ dispose(m); }
            if (timeout()){ break; }
            if(m->round == StartViewChange_ROUND && round == StartViewChange_ROUND && follower(n, view) == pid && collected_all_start_view_change(mbox,view) ) {
                break;
            }
            if(m->round == StartView_ROUND && round == StartView_ROUND && follower(n, view) == pid) {
                break;
            }
            if(m->round == DoViewChange_ROUND && round == StartViewChange_ROUND && primary(n, view) == pid) {
                break;
            }
            if(m->round == DoViewChange_ROUND && round == DoViewChange_ROUND && primary(n, view) == pid && collected_all_do_view_change(mbox,view) ) {
                break;
            }
        }
        if(m->view == view && m->round == StartViewChange_ROUND && round == StartViewChange_ROUND && follower(n, view) == pid && collected_all_start_view_change(mbox,view) ) {
            round = DoViewChange_ROUND;
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = DoViewChange_ROUND;
            send((void*)m, primary(n, view));
            round = StartView_ROUND;
         continue;
        }
        if(m->view == view && m->round == StartView_ROUND && round == StartView_ROUND && follower(n, view) == pid) {
            out(view);
            round = StartViewChange_ROUND;
            view++;
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = StartViewChange_ROUND;
            send((void*)m, to_all);
            continue;
        }
        if(m->view == view && m->round == DoViewChange_ROUND && round == StartViewChange_ROUND && primary(n, view) == pid) {
            round = DoViewChange_ROUND;
         continue;
        }
        if(m->view == view && m->round == DoViewChange_ROUND && round == DoViewChange_ROUND && primary(n, view) == pid && collected_all_do_view_change(mbox,view) ) {
            round = StartView_ROUND;
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = StartView_ROUND;
            send((void*)m, to_all);
            out(view);
            round = StartViewChange_ROUND;
            view++;
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = StartViewChange_ROUND;
            send((void*)m, to_all);
            continue;
        }
    }
    return 1;
}
