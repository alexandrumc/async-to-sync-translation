#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
typedef struct Msg {
 int round;
 int epoch;
} msg;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
int timeout();
int reset_timeout();
enum round_typ {FIRST_ROUND, SECOND_ROUND, AUX_ROUND} ;
msg* recv()
{
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) abort();
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
void send(msg* message, int pid);
int leader(int phase, int net_size)
{
    int res = phase % net_size;
    return res;
}
int main(int argc, char **argv)
{
    int n = argc;
    int to_all = n+1;
    int* log = (int*)malloc(100*sizeof(int));
    if(log==0) abort();
    enum round_typ round;
    int epoch, old_epoch;
    int pid;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    epoch = 0;
    old_epoch = epoch-1;
    round = AUX_ROUND;
    enum round_typ old_round = round;
    while(1)
    {
        old_epoch = epoch;
        round = FIRST_ROUND;
        old_round = round;
        if (pid == leader(epoch,n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
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
            if (m != NULL && m->epoch == epoch && m->round == FIRST_ROUND){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else mbox_new->size =1 ;
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
            old_round = round;
            round = SECOND_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if (m==0) abort();
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
                 if (m != NULL && m->epoch == epoch && m->round == SECOND_ROUND){
                                mbox_new = (list*) malloc(sizeof(list));
                                if(mbox_new==0) abort();
                                mbox_new->message =m;
                                if(mbox!=0)
                                    mbox_new->size = mbox->size + 1;
                                else mbox_new->size =1 ;
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
                    printf("\n%d", epoch);
                    int leader = leader(epoch,n);
             }
            if(mbox!=0) list_dispose(mbox);
        }
        old_epoch = epoch;
        epoch++;
        old_round = round;
        round = AUX_ROUND;
    }
    return 1;
}
