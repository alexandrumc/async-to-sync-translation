#include "stdlib.h"
#include<stdio.h>
#include<limits.h>

typedef struct Msg {
    int round;
    int phase;
    int estimate;
    int timestamp;
    int ack;
    int sender;
} msg;


/*@
 predicate msg_pred(struct Msg * msg) =
 msg == 0? true : msg->round |-> _ &*& msg->phase |-> _ &*& msg->sender |-> _ &*&
 msg->estimate |-> _ &*& msg->timestamp |-> _ &*& msg->ack |-> _ &*& malloc_block_Msg(msg);
 @*/

/*@
 predicate msg_pred_alloc1(struct Msg * msg, int phase, int round, int estimate, int timestamp, int ack, int sender) =
 msg->round |-> round &*& msg->phase |-> phase &*& msg->sender |-> sender &*&
 msg->estimate |-> estimate &*& msg->timestamp |-> timestamp &*& msg->ack |-> ack ;
 @*/

/*@
 predicate msg_pred_alloc2(struct Msg * msg) =
 msg->round |-> _ &*& msg->phase |-> _ &*& msg->sender |-> _ &*&
 msg->estimate |-> _ &*& msg->timestamp |-> _ &*& msg->ack |-> _ &*& malloc_block_Msg(msg);
 @*/


typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

/*@
 predicate msgs(list<struct Msg *> vs) = switch (vs) {
 case nil: return true;
 case cons(v, vs0): return msg_pred_alloc2(v) &*& msgs(vs0); };
 @*/


/*@
 // Predicate describing a complete linked list
 predicate list_pred(struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next 
 &*& list_pred(next)&*& malloc_block_Msg(msg) &*&   msg->round |-> ?round &*& msg->phase |-> ?phase &*& msg->sender |-> ?sender &*&
 msg->estimate |-> ?estimate &*& msg->timestamp |-> ?timestamp &*& msg->ack |-> ?ack ;
 @*/


/*@
 // Predicate describing a segment of a linked list from 'start' until
 // 'end' (not including 'end')
 predicate lseg_pred(struct List* start, struct List* end;) =
 start == end ?
 true
 :
 start->message |-> ?msg &*& start->size |-> ?size &*&  start->next |-> ?next &*& start != 0 &*& start != next 
 &*& malloc_block_List(start)&*& malloc_block_Msg(msg) &*&   msg->round |-> ?round &*& msg->phase |-> ?phase &*& msg->sender |-> ?sender &*&
 msg->estimate |-> ?estimate &*& msg->timestamp |-> ?timestamp &*& msg->ack |-> ?ack  &*& lseg_pred(next, end)  ;
 
 @*/

/*@
 // Lemma for adding (appending) a node to a list segment predicate
 lemma void lseg_append_node(struct List* lseg_start, struct List* lseg_end)
 
 requires lseg_pred(lseg_start, lseg_end) &*&
 lseg_end != 0 &*& lseg_end->message |-> ?msg &*& lseg_end->size |-> _ &*& lseg_end->next |-> ?next1  &*&
 malloc_block_List(lseg_end)&*& malloc_block_Msg(msg) &*& msg->round |-> ?round &*& msg->phase |-> ?phase &*& msg->sender |-> ?sender &*&
 msg->estimate |-> ?estimate &*& msg->timestamp |-> ?timestamp &*& msg->ack |-> ?ack &*&list_pred(next1);
 ensures lseg_pred(lseg_start, next1) &*& list_pred(next1);
 {
 open lseg_pred(lseg_start, lseg_end);
 if (lseg_start == lseg_end) {
 close lseg_pred(next1, next1);
 } else {
 lseg_append_node(lseg_start->next, lseg_end);
 }
 open list_pred(next1);
 close list_pred(next1);
 close lseg_pred(lseg_start, next1);
 }
 @*/

void dispose(msg* c)
//@ requires c->phase |-> _ &*& c->round |-> _ &*& c->estimate |-> _ &*&c->timestamp |-> _ &*&c->sender |-> _ &*&c->ack |-> _ &*&malloc_block_Msg(c);
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

int in();
//@requires true;
//@ensures true;

int leader(int phase, int net_size)
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;
{
    int res = phase % net_size;
    //@ assume(0<=res && res<net_size);
    return res;
}

/*@
 // Predicate describing the min of a complete linked list
 predicate eq_val_list_pred(int val, int val2, struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->phase |-> ?v &*& msg->round |-> ?r  &*& msg->estimate |-> _ &*& msg->timestamp |-> _ 
 &*& msg->ack |-> _ &*& msg->sender |-> _ &*& malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val== v &*&  val2== r &*& eq_val_list_pred(val,val2, next);
 @*/

/*@
 // Lemma for converting a list segment predicate describing a complete
 // list into a full list predicate.
 lemma void lseg_to_list(struct List *n)
 requires lseg_pred(n, 0);
 ensures list_pred(n);
 {
 open lseg_pred(n, 0);
 if (n != 0) {
 lseg_to_list(n->next);
 }
 close list_pred(n);
 }
 @*/

msg* max_timestamp(struct List* mbox)
//@ requires list_pred(mbox) &*& mbox!=0;
//@ ensures (list_pred(mbox) &*& mbox!=0 &*& result->phase |-> ?p &*& result->round |-> ?r  &*& result->estimate |-> ?e &*& result->timestamp |-> _ &*& result->ack |-> _ &*& result->sender |-> _ &*& malloc_block_Msg(result));
// max_lseg_pred(result,mbox,0);
{
    
    struct List *current = mbox;
    //@open (list_pred(mbox));
    msg *max = (msg*)malloc(sizeof(msg));
    if (max ==0) {
        abort();
    }
    max->estimate = mbox->message->estimate;
    max->phase =  mbox->message->phase;
    max->round =  mbox->message->round;
    max->sender =  mbox->message->sender;
    max->ack =  mbox->message->ack;
    max->timestamp =  mbox->message->timestamp;
    
    current = mbox;
    //@close lseg_pred(mbox,current);
    //@close list_pred(current);
    //@close msg_pred_alloc2(max);
    ////@close(max_lseg_pred(max,mbox,current));
    ////@close(list_pred(current));
    struct List* current0  = NULL;
    
    while (current != NULL)
        //@ invariant (lseg_pred(mbox,current) &*& list_pred(current) &*& msg_pred_alloc2(max)); 
    {
        //@open lseg_pred(mbox,current);
        //@open list_pred(current);
        //@open msg_pred_alloc2(max);
        if(current->message->timestamp > max->timestamp){
            max->estimate = current->message->estimate;
            max->phase =  current->message->phase;
            max->round =  current->message->round;
            max->timestamp = current->message->timestamp;
            max->sender =  current->message->sender;
            max->ack = current->message->ack;	
        }
        //@close msg_pred_alloc2(max);
        current0 = current->next; 
        //@lseg_append_node(mbox,current);
        current = current0;    
    }
    
    //@assert lseg_pred(mbox,0);
    //@ lseg_to_list(mbox);
    //@open msg_pred_alloc2(max);
    return max;
}


/*@
 // Lemma for converting a full eq_val_ list predicate into a list predicate
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




msg* recv()
//@ requires emp;
//@ ensures result->round |-> ?v1 &*& result->phase |-> ?v &*& result->estimate |-> _ &*& result->timestamp |-> _ &*& result->sender |-> _&*& result->ack |-> _ &*& malloc_block_Msg(result) &*& INT_MIN <v &*& v < INT_MAX;
{
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) {
        abort();
    }
    //@ assume(m->phase > INT_MIN && m->phase < INT_MAX);
    return m;
}


void send(msg* message, int pid);
//@ requires true;
//@ ensures true;

void out(int pid, int est);
//@ requires true;
//@ ensures true;

int timeout();
//@ requires emp;
//@ ensures emp;

enum round_typ {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, FOURTH_ROUND, AUX_ROUND} ;

int main(int argc, char**argv) //@ : main
//@ requires true;
//@ ensures true;
{
    int state = 0;
    
    int round = AUX_ROUND;
    //@ int old_round = round;
    int myid = 0;
    int timestamp = 0;
    
    int phase = 0;
    //@ int old_phase = -1;
    
    int estimate = in();
    int leader = 0;
    
    int n = argc; 
    int to_all = n+1;
    
    
    
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    
    
    int retry;
    int timeout;
    int ack = 0;
    
    msg* m = NULL;
    
    volatile int random;
    
    
    while(true)
        //@invariant ((old_phase+1==phase) && (round==AUX_ROUND));
    {
        //@ old_round = round;
        round = FIRST_ROUND;
        
        //@ assert ((old_phase+1==phase) || (old_phase==phase && old_round <=round));
        ////@ assert (old_round <=round);
        
        //@ old_phase = phase;
        //@ old_round = round;
        //@ assert ((old_phase+1==phase) || (old_phase==phase && old_round <=round));
        
        m = NULL;
        m = (msg*)malloc(sizeof(msg));
        if (m == 0) {
            abort();
        }
        
        m->phase = phase;
        m->round = round;
        m->estimate = estimate;
        m->sender = myid;
        m->timestamp = timestamp;
        
        leader = leader(phase,n);
        send(m,leader);
        
        dispose(m);
        m = NULL;
        mbox = NULL;
        
        if (myid == leader) {
            //@ close list_pred(mbox);
            if (mbox!= 0) { list_dispose(mbox); }
            mbox = NULL;
            
            while (1)
                //@ invariant eq_val_list_pred(phase,round,mbox);
            {
                //@open eq_val_list_pred(phase,round,mbox);
                m = recv();
                if (m->round == 1 && m->phase == phase && m->round == round) {
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
                }else{free(m);}
                
                if (timeout()) break;
                else if (mbox!=0 && mbox->size >= (n + 1) / 2)
                    break;
                //@ close eq_val_list_pred(phase,round,mbox);
            }
            
            if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                //@ close eq_val_list_pred(phase,round,mbox);
                //@ eq_val_list_pred_to_list_pred_lemma(mbox);
                m = max_timestamp(mbox);
                estimate = m->estimate;
                free(m);
                m=NULL;
            }else{
                //@ close eq_val_list_pred(phase,round,mbox);
                //@ eq_val_list_pred_to_list_pred_lemma(mbox);
                list_dispose(mbox);
                mbox = NULL;
            }
        }
        
        //@close list_pred(mbox);
        //@ assert list_pred(mbox);
        
        if (mbox!= 0) { list_dispose(mbox); }
        mbox = NULL;
        
        
        //@  old_round = round;
        round = SECOND_ROUND;
        ////@ assert (old_round <=round);
        //@ assert ((old_phase+1==phase) || (old_phase==phase && old_round <=round));
        
        if (myid == leader) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            send(m,to_all);
            dispose(m);
            m = NULL;
        }
        
        while (1)
            //@ invariant eq_val_list_pred(phase,round,mbox);
        {
            //@open eq_val_list_pred(phase,round,mbox);
            m = recv();
            if (m->phase == phase && m->round == SECOND_ROUND) {
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
            }else{free(m);}
            
            //@ assert eq_val_list_pred(phase,round,mbox);
            if (timeout()) break;
            else if (mbox!=0 && mbox->message->sender ==leader )
                break;
            
            //@ close eq_val_list_pred(phase,round,mbox);
        }
        
        if (mbox!=0 && mbox->message->sender ==leader ) {
            estimate = mbox->message->estimate;
            timestamp = phase;
            //@ close eq_val_list_pred(phase,round,mbox);
            //@ eq_val_list_pred_to_list_pred_lemma(mbox);
        }else{	
            //@ close eq_val_list_pred(phase,round,mbox);
            //@ eq_val_list_pred_to_list_pred_lemma(mbox);
            list_dispose(mbox);
            mbox = NULL;
        }
        
        //@ old_round = round;
        round = THIRD_ROUND;
        //@ assert ((old_phase+1==phase) || (old_phase==phase && old_round <=round));
        
        if (timestamp == phase) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            send(m,leader);
            dispose(m);
            m = NULL;
        }
        
        //@close list_pred(mbox);
        if (mbox!= 0) { list_dispose(mbox); } 
        mbox = NULL;
        if (myid == leader) {
            while (1)
                //@ invariant eq_val_list_pred(phase,round,mbox);
            {
                //@open eq_val_list_pred(phase,round,mbox);
                m = recv();
                if (m->phase == phase && m->round == round) {
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
                }else{free(m);}
                
                
                if (timeout()) break;
                else if (mbox!=0 && mbox->size >= (n + 1) / 2)
                    break;
                
                //@ close eq_val_list_pred(phase,round,mbox);
            }
            
            if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                //@ close eq_val_list_pred(phase,round,mbox);
                //@ eq_val_list_pred_to_list_pred_lemma(mbox);
                ack = 1;
                
            }else{
                //@ close eq_val_list_pred(phase,round,mbox);
                //@ eq_val_list_pred_to_list_pred_lemma(mbox);
                list_dispose(mbox);
                mbox = NULL;
            }
        }
        
        //@   old_round = round;
        round = FOURTH_ROUND;
        //@ assert ((old_phase+1==phase) || (old_phase==phase && old_round <=round));
        
        if (myid == leader && ack == 1) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->ack = ack;
            send(m,to_all);
            dispose(m);
            m = NULL;
        }
        
        //@close list_pred(mbox);
        //@ assert list_pred(mbox);
        
        
        if (mbox!= 0) { list_dispose(mbox); }
        
        mbox = NULL;
        
        while (1) //@ invariant eq_val_list_pred(phase,round,mbox);
        {
            //@ assert eq_val_list_pred(phase,round,mbox);
            //@open eq_val_list_pred(phase,round,mbox);
            m = recv();
            if (m->round == FOURTH_ROUND && m->phase == phase) {
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
            }else{free(m);}
            
            
            if (timeout()) break;
            else if (mbox!=0 )break;
            //@ assert eq_val_list_pred(phase,round,mbox);
            
            //@ close eq_val_list_pred(phase,round,mbox);
        }
        
        
        
        if (mbox!=0)
        {
            //@ assert eq_val_list_pred(phase,round,mbox);
            estimate = mbox->message->estimate;
            state = 1;
            out(myid,estimate);
            //@ close eq_val_list_pred(phase,round,mbox);
            //@ eq_val_list_pred_to_list_pred_lemma(mbox);
            
        }	
        
        //@ assert list_pred(mbox);
        //@close list_pred(mbox);
        
        if (mbox!= 0) { list_dispose(mbox); }
        mbox = NULL; 
        
        //@  old_phase = phase;
        
        phase = phase + 1;
        round = AUX_ROUND;
    }
    return 1;
}

//}
