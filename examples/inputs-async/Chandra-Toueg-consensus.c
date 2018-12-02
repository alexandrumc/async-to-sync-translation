#include "stdlib.h"
#include<stdio.h>
#include<limits.h>

//
//  CT_terminating_v01.c
//  
//
//  Created by Cezara on 14/10/2018.
//




typedef struct Msg {
    int round;
    int phase;
    int estimate;
    int timestamp;
    int ack;
    int sender;
} msg;

/*@ predicate tag_strict_leq(int p1, int r1, int p2, int r2) =  r2 == 3 ? true: (r1==4 ? false : ((p1+1 == p2) || (p1==p2 && r1<=r2)));
 @*/

typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;

enum round_typ {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, FOURTH_ROUND, AUX_ROUND} ;

void out(int v1, int v2);
//@ requires true;
//@ ensures true;

int in();
//@ requires true;
//@ ensures true;

int timeout();
//@ requires emp;
//@ ensures emp;
int reset_timeout();
//@ requires emp;
//@ ensures emp;
int rand_bool();
//@ requires emp;
//@ ensures 1<= result && 0<= result ;


msg *recv();
//@ requires emp;
/*@ ensures result->round |-> ?r &*& result->phase |-> ?p &*&
 result	->estimate |-> ?rsp &*& result->timestamp |-> ?c &*&
 result->ack |-> ?pay &*& result->sender |-> ?s &*&
 malloc_block_Msg(result) &*& INT_MIN < p &*& p < INT_MAX;
 @*/

msg* max_timestamp(struct List* mbox);
//@ requires true;
//@ ensures (result->phase |-> ?p &*& result->round |-> ?r  &*& result->estimate |-> ?e &*& result->timestamp |-> _ &*& result->ack |-> _ &*& result->sender |-> _ &*& malloc_block_Msg(result));

int all_ack(struct List* mbox);
//@ requires true;
//@ ensures true;

void dispose(msg *c);
//@ requires c->phase |-> _ &*& c->round |-> _ &*& c->estimate |-> _ &*& c->ack |-> _ &*& c->timestamp |-> _ &*& c->sender |-> _ &*& malloc_block_Msg(c);
//@  ensures emp;


void list_dispose(struct List *l);
//@ requires list_pred(l);
//@ ensures emp;


void send(msg *message, int pid);
//@ requires true;
//@ ensures true;

int leader(int phase, int net_size);
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;

int all_agree(struct List *l);

int main(int argc, char**argv) //@ : main
//@ requires true;
//@ ensures true;
{
    int state = 0;
    
    int round = 0;
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
    
    int ack = 0;
    
    
    msg* m = NULL;
    round = FIRST_ROUND;
    phase = 1;
    
    ack = rand_bool();
    
    while(1)
        //@ invariant old_phase < phase && round == FIRST_ROUND;
    {
        
    
    round = FIRST_ROUND;
    //@ old_phase = phase;
    //@ old_round = round;
    
    leader = leader(phase,n);
    
    m = NULL;
    m = (msg *) malloc(sizeof(msg));
    if (m == 0) {abort();}
    
    m->phase = phase;
    m->round = round;
    m->estimate = estimate;
    m->sender = myid;
    m->timestamp = timestamp;
    m->ack = 0;
    
    //@assert(m->phase == phase && m->round == round);
    send(m, leader);
    
    dispose(m);
    m = NULL;
    mbox = NULL;
    
    
    
    if (myid == leader) {
        //@ close list_pred(mbox);
        while (true)
            //@ invariant eq_val_list_pred(phase,round,mbox);
        {
            //@open eq_val_list_pred(phase,round,mbox);
            m = recv();
            
            if (m!= NULL && m->round == FIRST_ROUND && m->phase == phase) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort();}
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else{ mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else if (m!=NULL && m->round == FOURTH_ROUND ){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort();}
                mbox_new->message = m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else{ mbox_new->size =1 ; }
                mbox_new->next = mbox;
                mbox = mbox_new;
                break;
            } else {free(m);};
            
            if (mbox!=NULL && mbox->message != NULL &&mbox->message->round ==FOURTH_ROUND){break;}
            if(mbox!=NULL && mbox->size >= (n + 1) / 2) {
                //@ close eq_val_list_pred(phase,round,mbox);
                break;
            }
            
        }
        //@ close mbox_CT_mbox(mbox, phase, round);
        //@ open list_pred(NULL);
        //@ assert mbox_CT_mbox(mbox, phase, round);
        
        //@ open mbox_CT_mbox(mbox, phase, round);
        
        if(mbox!=0 && mbox->message!=NULL)
         if(mbox->message->round == FOURTH_ROUND){
                //@old_round = round;
                round = FOURTH_ROUND;
                
                //@ close  max_tag_of_mbox(mbox, phase, round);
                //@ assert max_tag_of_mbox(mbox, phase, round);
                //@ open max_tag_of_mbox(mbox, phase, round);
                
                estimate = mbox->message->estimate;
                state = 1;
                out(myid, estimate);

                //@ close mbox_CT_mbox(mbox, phase, old_round);
                //@ leak mbox_CT_mbox(mbox, phase, old_round);
                
                //list_dispose(mbox);
                mbox = NULL;
                
                break;
                round = AUX_ROUND;
            }else {
               if (mbox->size >= (n + 1) / 2) {
                    //@ assert eq_val_list_pred( phase, round,mbox);
                    m = max_timestamp(mbox);
                    estimate = m->estimate;
                    free(m);
                    m=NULL;
                    
                }
            
        }
        //@ close mbox_CT_mbox(mbox, phase, round);
        //@leak mbox_CT_mbox(mbox, phase, round);
        // list_dispose(mbox);
        mbox = NULL;
    }
    
    //@ old_round = round;
    round = SECOND_ROUND;
    //@ close tag_strict_leq(old_phase,old_round,phase,round);
    //@assert tag_strict_leq(old_phase,old_round,phase,round);
    //@ open  tag_strict_leq(old_phase,old_round,phase,round);
    
    
    if (myid == leader) {
        m = (msg *) malloc(sizeof(msg));
        if(m==0) {abort();}
        m->sender = myid;
        m->phase = phase;
        m->round = round;
        m->estimate = estimate;
        m->timestamp = timestamp;
        
        //@assert(m->phase == phase && m->round == round);
        send(m,to_all);
        dispose(m);
        m = NULL;
    }
    //close mbox_CT_mbox(mbox, phase, round);
    //leak mbox_CT_mbox(mbox,phase,round);
    
    //list_dispose(mbox);
    mbox = NULL;
    
    //@ close list_pred(mbox);
    while (1)
        //@ invariant eq_val_list_pred(phase,round,mbox);
    {
        //@open eq_val_list_pred(phase,round,mbox);
        m = recv();
        if (m!= NULL && m->phase == phase && m->round == SECOND_ROUND) {
            mbox_new = (list*) malloc(sizeof(list));
            if(mbox_new==0) {abort();}
            mbox_new->message =m;
            if(mbox!=0){mbox_new->size = mbox->size + 1; }
            else{mbox_new->size =1 ;}
            mbox_new->next = mbox;
            mbox = mbox_new;
        } else {
            if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort();}
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1; }
                else {mbox_new->size = 1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            } else {free(m);}
        }
        if(timeout()) {break; }
        if (mbox!=NULL && mbox->message != NULL && (mbox->message->sender ==leader || mbox->message->round ==FOURTH_ROUND )){
            
            break;
        }
        //@ close eq_val_list_pred(phase,round,mbox);
    }
    
    //@ close mbox_CT_mbox(mbox, phase, round);
    //@ open list_pred(NULL);
    //@ assert mbox_CT_mbox(mbox, phase, round);
    
    //@ open mbox_CT_mbox(mbox, phase, round);
    
    
    if(mbox!=0 && mbox->message!=NULL){
        
        if( mbox->message->round == FOURTH_ROUND){
            //@ old_round =  round;
            round = FOURTH_ROUND;
            //@ close max_tag_of_mbox(mbox, phase, round);
            //@ assert max_tag_of_mbox(mbox, phase, round);
            //@ open max_tag_of_mbox(mbox, phase, round);
            estimate = mbox->message->estimate;
            state = 1;
            out(myid, estimate);
            
            //@ close mbox_CT_mbox(mbox, phase, old_round);
            //@ leak mbox_CT_mbox(mbox, phase, old_round);
            
            //list_dispose(mbox);
            mbox = NULL;
            
            break;
        } else{
            //@ assert eq_val_list_pred( phase, round,mbox);
            estimate = mbox->message->estimate;
            timestamp = phase;
            ack = 1;
        }
    }
    else {ack = -1;}

    
    //@ close mbox_CT_mbox(mbox, phase, round);
    //@ leak mbox_CT_mbox(mbox, phase, round);
    //list_dispose(mbox);
    mbox = NULL;
    
    
    
    //@ old_round = round;
    round = THIRD_ROUND;
    //@ close tag_strict_leq(old_phase,old_round,phase,round);
    //@assert tag_strict_leq(old_phase,old_round,phase,round);
    //@ open tag_strict_leq(old_phase,old_round,phase,round);
    
    
    m = (msg *) malloc(sizeof(msg));
    if(m==0) {abort();}
    m->sender = myid;
    m->phase = phase;
    m->round = round;
    m->estimate = estimate;
    m->timestamp = timestamp;
    m->ack = ack;
    
    //@assert(m->phase == phase && m->round == round);
    send(m,leader);
    dispose(m);
    m = NULL;
    mbox = NULL;
    
    
    if (myid == leader) {

        //@ close list_pred(mbox);

        while (1)
            //@ invariant eq_val_list_pred(phase,round,mbox);
        {
            //@open eq_val_list_pred(phase,round,mbox);
            m = recv();
            if (m!= NULL && m->round == round && m->phase == phase) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort();}
                mbox_new->message =m;
                if(mbox!=0) {mbox_new->size = mbox->size + 1; }
                else { mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else  if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {abort(); }
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else{ mbox_new->size =1 ;}
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else {free(m);}
            
            if (mbox!=NULL && mbox->message != NULL && mbox->message->round ==FOURTH_ROUND){break;}
            if(mbox!=NULL && mbox->size >= (n + 1) / 2) {
                //@ close eq_val_list_pred(phase,round,mbox);
                break;
                round = AUX_ROUND;
            }
        }
        //@ close mbox_CT_mbox(mbox, phase, round);
        //@ open list_pred(NULL);
        //@ assert mbox_CT_mbox(mbox, phase, round);
        
        
        
        //@ open mbox_CT_mbox(mbox, phase, round);
        
        if(mbox!=0 && mbox->message!=NULL &&mbox->message->round == FOURTH_ROUND){
                //@old_round = round;
                round = FOURTH_ROUND;
                
                //@ close  max_tag_of_mbox(mbox, phase, round);
                //@ assert max_tag_of_mbox(mbox, phase, round);
                //@ open max_tag_of_mbox(mbox, phase, round);
                
                estimate = mbox->message->estimate;
                state = 1;
                
                out(myid, estimate);
                //@ close mbox_CT_mbox(mbox, phase, old_round);
                //@ leak mbox_CT_mbox(mbox, phase, old_round);
                
                //list_dispose(mbox);
                mbox = NULL;
                break;
                round = AUX_ROUND;
        }else{
            //@ assume (round == THIRD_ROUND);
            //@ assume (mbox->message->round == round); 
            //@ assert eq_val_list_pred( phase, round,mbox);
            if(all_ack(mbox)) { ack =1;}
            else {ack = -1;}
        }
        //@ close mbox_CT_mbox(mbox, phase, round);
        //@leak mbox_CT_mbox(mbox, phase, round);
        //list_dispose(mbox);
        mbox = NULL;
    }
    
    
    //@ old_phase = phase;
    //@ old_round = round;
    round = FOURTH_ROUND;
    //@ close tag_strict_leq(old_phase,old_round,phase,round);
    //@assert tag_strict_leq(old_phase,old_round,phase,round);
    //@ open tag_strict_leq(old_phase,old_round,phase,round);
    
    
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
        
        //@assert(m->phase == phase && m->round == round);
        send(m,to_all);
        dispose(m);
        m = NULL;
        
    }
    
    //@close mbox_CT_mbox(mbox,phase,round);
    //@leak mbox_CT_mbox(mbox,phase,round);
    //list_dispose(mbox);
    mbox = NULL;
    //@ close eq_max_val_list_pred(mbox);
    while (1)
        //@ invariant eq_max_val_list_pred(mbox);
    {
        //@open eq_max_val_list_pred(mbox);
        m = recv();
        if (m!= NULL && m->round == FOURTH_ROUND) {
            mbox_new = (list*) malloc(sizeof(list));
            if(mbox_new==0) {abort();}
            mbox_new->message =m;
            if(mbox!=0){mbox_new->size = mbox->size + 1;}
            else{mbox_new->size =1 ;}
            mbox_new->next = mbox;
            //@ close eq_max_val_list_pred(mbox);
            mbox = mbox_new;
            //@ close eq_max_val_list_pred(mbox);
            break;
            
        }else{free(m);}
        
        if (timeout()){ //@ close eq_max_val_list_pred(mbox);
            break; }
        //@ close eq_max_val_list_pred(mbox);
    }
    ////@ assert mbox_tag_geq(mbox, phase, round);
    
    
    //@  assert  eq_max_val_list_pred(mbox);
    
    if (mbox!=0)
    {
        
        //@ assert eq_max_val_list_pred(mbox);
        
        
        ////@ open eq_max_val_list_predmax_tag_of_mbox(mbox, phase, round);
        ////@ assert max_tag_of_mbox(mbox, phase, round);
        ////@ close max_tag_of_mbox(mbox, phase, round);
        //@ open eq_max_val_list_pred(mbox);
        estimate = mbox->message->estimate;
        state = 1;
        out(myid,estimate);
        
        //@ close eq_max_val_list_pred(mbox);
        //@ leak eq_max_val_list_pred(mbox);
        //list_dispose(mbox);
        mbox = NULL;
        break;
        round = AUX_ROUND;
        
    }else{
    //@ leak eq_max_val_list_pred(mbox);
    }
    
    
    
    phase = phase + 1;
    //@old_round = round;
    round = FIRST_ROUND;
    
    
    //@ close tag_strict_leq(old_phase,old_round,phase,round);
    //@assert tag_strict_leq(old_phase,old_round,phase,round);
    //@ open tag_strict_leq(old_phase,old_round,phase,round);
    
}



return 1;
}
/*@
 // Predicate describing a complete linked list
 predicate list_pred(struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next
 &*& list_pred(next)&*& malloc_block_Msg(msg) &*&   msg->round |-> ?round &*& msg->phase |-> ?phase &*& msg->sender |-> ?sender &*&
 msg->estimate |-> ?estimate &*& msg->timestamp |-> ?timestamp &*& msg->ack |-> ?ack ;
 
 predicate eq_val_list_pred(int val, int val2, struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->phase |-> ?v &*& msg->round |-> ?r  &*& msg->estimate |-> _ &*& msg->timestamp |-> _
 &*& msg->ack |-> _ &*& msg->sender |-> _ &*& malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val== v &*&  val2== r &*& eq_val_list_pred(val,val2, next);
 
 predicate mbox_CT_mbox (struct List *mbox, int phase, int round) =
 mbox == NULL ? eq_val_list_pred(phase,round, NULL) :
 mbox->next|-> ?next &*& mbox->message |-> ?msg &*& mbox->size |-> ?size &*&  mbox != next &*& malloc_block_List(mbox) &*&
 msg->round |-> ?r &*& msg->phase |-> ?p &*& msg->sender |-> ?s &*& msg->ack |-> ?e &*&
 msg->timestamp |-> ?t &*& msg->estimate |-> ?a &*& malloc_block_Msg(msg) &*&
 (r == FOURTH_ROUND) ?
 eq_val_list_pred(phase,round, next) :
 (phase == p &*& round ==r
 &*& eq_val_list_pred(phase,round, next));
 
 predicate  max_tag_of_mbox(struct List *mbox, int phase, int round) =
 (round == FOURTH_ROUND ) ? true : false;
 
 predicate eq_max_val_list_pred (struct List * mbox) =
 mbox == 0 ? true :
 mbox->next|-> ?next &*& mbox->message |-> ?msg &*& mbox->size |-> ?size &*&  mbox != next &*& malloc_block_List(mbox) &*&
 msg->round |-> ?r &*& msg->phase |-> ?p &*& msg->sender |-> ?s &*& msg->ack |-> ?e &*&
 msg->timestamp |-> ?t &*& msg->estimate |-> ?a &*& malloc_block_Msg(msg) &*&
 (r == FOURTH_ROUND) ? eq_max_val_list_pred(next) :false;
 @*/
