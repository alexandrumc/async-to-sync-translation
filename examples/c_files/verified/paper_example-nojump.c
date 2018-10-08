#include "stdlib.h"
#include<stdio.h>
#include<limits.h>

typedef struct Msg {
	int round;
	int epoch;
} msg;


/*@
 predicate msg_pred(struct Msg * msg) =
 msg == 0? true : msg->round |-> _ &*& msg->epoch |-> _ &*& malloc_block_Msg(msg);
 @*/


typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

/*@
 // Predicate describing a complete linked list
 predicate list_pred(struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next 
 &*& msg->round |-> _ &*& msg->epoch |-> _ &*& malloc_block_Msg(msg) &*& list_pred(next);
 @*/


/*@
 // Predicate describing a segment of a linked list from 'start' until
 // 'end' (not including 'end')
 predicate lseg_pred(struct List* start, struct List* end;) =
 start == end ?
 true
 :
 start->message |-> ?msg &*& start->size |-> ?size &*&  start->next |-> ?next &*& start != 0 &*& start != next 
 &*& malloc_block_List(start)&*& msg->round |-> _ &*& msg->epoch |-> _ &*& malloc_block_Msg(msg)  &*& lseg_pred(next, end)  ;
 @*/


/*@
 // Lemma for converting a full min list predicate into a list predicate
 lemma void eq_val_list_to_list_pred_lemma(int val, int val2, struct List *l)
 requires eq_val_list_pred(val,val2, l);
 ensures list_pred(l);
 {
 open eq_val_list_pred(val,val2, l);
 if (l != 0) {
 eq_val_list_to_list_pred_lemma(val,val2, l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open list_pred(l->next);
 close list_pred(l->next);
 }
 close list_pred(l);
 }
 @*/


/*@
 // Predicate describing the min of a complete linked list
 predicate eq_val_list_pred(int val, int val2, struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?v &*& msg->round |-> ?r &*& malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val== v &*&  val2== r &*& eq_val_list_pred(val,val2, next);
 @*/



/*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void eq_val_list_pred_to_list_pred_lemma(struct List *l)
 requires eq_val_list_pred( ?val, ?val2, l);
 ensures list_pred(l);
 {
 open eq_val_list_pred(val,val2, l);
 if (l != 0) {
 
 eq_val_list_pred_to_list_pred_lemma(l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open list_pred(l->next);
 close list_pred(l->next);
 close list_pred(l);
 }
 else close list_pred(0);
 }
 @*/


bool timeout();
//@ requires emp;
//@ ensures emp;
bool reset_timeout();
//@ requires emp;
//@ ensures emp;


enum round_typ {FIRST_ROUND, SECOND_ROUND, AUX_ROUND} ;

msg* recv()
//@ requires emp;
//@ ensures result->round |-> ?v1 &*& result->epoch |-> ?v &*& malloc_block_Msg(result) &*& INT_MIN <v &*& v < INT_MAX;
{
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) abort();
    //@ assume(m->epoch > INT_MIN && m->epoch < INT_MAX);
    return m;
}


void dispose(msg* c)
//@ requires c->epoch |-> _ &*& c->round |-> _ &*& malloc_block_Msg(c);
//@ ensures emp;
{
    free(c);
}


void list_dispose(struct List *l)
//@ requires list_pred(l);
//@ ensures emp;
{
    //@ open list_pred(l);
    struct List* current = l;
    while(current != 0)
        //@ invariant list_pred(current);
    {
        //@ open list_pred(current);
        struct List* oldcurrent = current;
        msg *m = current->message;
        current = current->next;
        free(m);
        free(oldcurrent);
    }
}



void dispose_list(list* c)
//@ requires  c->message |-> _ &*& c->next |-> _ &*& c->size |-> _ &*& malloc_block_List(c);
//@ ensures emp;
{
    free(c);
}

void send(msg* message, int pid);
//@ requires true;
//@ ensures true;

int leader(int phase, int net_size)
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;
{
    int res = phase % net_size;
    //@ assume(0<=res && res<net_size);
    return res;
}


int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
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


    round = FIRST_ROUND;
    enum round_typ old_round = round;
    while(1)
        //@ invariant ((old_epoch+1==epoch && round == FIRST_ROUND));
    {
        old_epoch = epoch;
        old_round = round;
        //@ assert ((old_epoch+1==epoch) || (old_epoch==epoch && old_round <=round));
        if (pid == leader(epoch,n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->epoch = epoch;
            m->round = FIRST_ROUND;
            //@assert(m->epoch == epoch && m->round == round);
            
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        reset_timeout();
        mbox = NULL;
        while(true)
            //@ invariant eq_val_list_pred(epoch,round,mbox);
        {
            //@open eq_val_list_pred(epoch,round,mbox);
            m = recv();
            if (m != NULL && m->epoch == epoch && m->round == FIRST_ROUND){
                
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
               
            }
            else free(m);
            if (timeout()) break;
                else if(mbox != NULL && mbox->size ==1){
                break;
            }
            //@ close eq_val_list_pred(epoch,round,mbox);
        }
        //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));
        if(mbox!=NULL){
            //@assert eq_val_list_pred(epoch,round,mbox);
        
       
            old_round = round;
            round = SECOND_ROUND;
            //@ assert ((old_epoch+1==epoch) || (old_epoch==epoch && old_round <=round));

            m = (msg *) malloc(sizeof(msg));
            if (m==0) abort();
            m->epoch = epoch;
            m->round = SECOND_ROUND;
            //@assert(m->epoch == epoch && m->round == round);
            send(m, to_all);
            
            dispose(m);
            m = NULL;
            //@ eq_val_list_to_list_pred_lemma(old_epoch, old_round, mbox);
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            while(true)
            //@ invariant eq_val_list_pred(epoch,round,mbox);
            {
                //@ open eq_val_list_pred(epoch,round,mbox);
                 m = recv();
                 if (m != NULL && m->epoch == epoch && m->round == SECOND_ROUND){
            
                                mbox_new = (list*) malloc(sizeof(list));
                                if(mbox_new==0) abort();
                                mbox_new->message =m;
                                if(mbox!=0)
                                    mbox_new->size = mbox->size + 1;
                                else  mbox_new->size =1 ;
                                mbox_new->next = mbox;
                                mbox = mbox_new;
            
                    }
                    else free(m);
                    if (timeout()) break;
                    else if(mbox != NULL && mbox->size > n/2){
                                break;
                    }

                    //@ close eq_val_list_pred(epoch,round,mbox);
            }
            
            
            if (mbox != NULL && mbox->size > n/2){
                    //@ assert eq_val_list_pred(epoch,round,mbox);
                    printf("\n%d", epoch);
                    int leader = leader(epoch,n);
                    
                   // log[epoch] = leader;
                    //out(epoch, leader());
             }
        	
           
             //@ close eq_val_list_pred(epoch,round,mbox);
             //@ eq_val_list_pred_to_list_pred_lemma(mbox);
            
            if(mbox!=0) list_dispose(mbox);
        }
       
        
        old_epoch = epoch;
        epoch++;
        old_round = round;
        round = FIRST_ROUND;
        
    }
    return 1;
}


