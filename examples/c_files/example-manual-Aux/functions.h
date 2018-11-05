//
//  functions.h
//  
//
//  Created by Cezara on 13/10/2018.
//



#ifndef functions_h
#define functions_h

#include <stdio.h>
#include "stdlib.h"
#include <stdbool.h>




typedef struct Msg {
    int round; //round
    int phase;  //phase
    int payload;
    int response;
    int sender;
    int commit;
} msg;


typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

void out(int v1, int v2);
//@ requires true;
//@ ensures true;

int in();
//@ requires true;
//@ ensures true;



bool timeout();
//@ requires emp;
//@ ensures emp;
bool reset_timeout();
//@ requires emp;
//@ ensures emp;
int rand_bool();
//@ requires emp;
//@ ensures 1<= result && 0<= result ;



msg* recv();
//@ requires emp;
/*@ ensures result->round |-> ?r &*& result->phase |-> ?p &*&
            result->response |-> ?rsp &*& result->commit |-> ?c &*&
            result->payload |-> ?pay &*& result->sender |-> ?s &*&
    malloc_block_Msg(result) &*& INT_MIN < p &*& p < INT_MAX;
@*/

void dispose(msg* c);
/*@
 requires c->phase |-> _ &*& c->round |-> _ &*&
 c->response |-> _ &*& c->commit |-> _ &*&
 c->payload |-> _ &*& c->sender |-> _ &*& malloc_block_Msg(c);
 @*/
 //@ensures emp;


void list_dispose(struct List *l);
//@ requires lseg(l,0, ?v) &*& foreach(v,alloc_list());
//@ ensures emp;

void list_dispose_no_data(struct List *l);
void dispose_list(list* c);

void send(msg* message, int pid);
//@ requires true;
//@ ensures true;

int leader(int phase, int net_size);
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;

int all_agree(struct List* l);
//@ requires lseg(l,0, ?v)&*& foreach(v,alloc_list());
//@ ensures lseg(l,0, v) &*& foreach(v,alloc_list()) &*& 0 <= result &*& result<=1;



/*@
predicate list(struct List* n; list<struct Msg *> v) =
 n == 0 ? (v == nil) :
           n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next &*& list(next, ?vnext) &*& v == cons(msg,vnext);
 
predicate lseg(struct List* from, struct List* to, list<struct Msg *> v)
requires from == to ? v == nil :
                      from->next |-> ?next &*& from->message |-> ?msg &*& from->size |-> ?size &*&
                      malloc_block_List(from) &*& lseg(next, to, ?nextv) &*& v == cons(msg,nextv);
 
 @*/
 
 /*@ predicate_ctor alloc_list()(struct Msg *msg) = msg == 0 ? true : msg->round |-> ?r &*& msg->phase |-> ?p &*&
 								  msg->sender |-> ?s &*& msg->response |-> ?e &*& 
 								  msg->payload |-> ?t &*& msg->commit |-> ?a &*& 
 								  malloc_block_Msg(msg);
  
  
  @*/


// Predicate: all messages tags are SMALLER than the input tag
/*@
 predicate_ctor geq_list(int phase, int round)(struct Msg *msg) =
 msg == 0 ? true :
            msg->round |-> ?r &*& msg->phase |-> ?p &*& msg->sender |-> ?s &*& msg->response |-> ?e &*&
            msg->payload |-> ?t &*& msg->commit |-> ?a &*& malloc_block_Msg(msg)&*&
            (phase > p) ? true : ((phase == p) &*& (round >= r) );
 
 lemma void lemma_geq_list_to_alloc_list(list<struct Msg *> v, int phase, int round)
  requires foreach(v,geq_list(phase,round));
  ensures foreach(v,alloc_list());
 {
 open foreach(v,geq_list(phase,round));
 if (v == nil) {
        close foreach(nil,alloc_list());
 }else{
 	open geq_list(phase,round)(head(v));
 	close alloc_list(head(v));
        lemma_geq_list_to_alloc_list(tail(v),phase,round);
        open foreach(tail(v),alloc_list());
        close foreach(tail(v),alloc_list());
        
        close foreach(cons(head(v),tail(v)),alloc_list());
    }
 }
  predicate mbox_tag_GEQ(struct List *mbox, int phase, int round) =
  lseg(mbox,0,?v) &*& foreach(v,geq_list(phase,round));
@*/

/*@
predicate stag_is_max_of_mbox(struct List* mbox, int phase, int round) = lseg(mbox,0,?v) &*& foreach(v,geq_list(phase,round));
 
 
 lemma void lemma_EQ_list_to_GEQ_list(struct List* mbox, int phase, int round)
 requires lseg(mbox,0, ?v) &*& foreach(v,eq_list(phase,round));
 ensures lseg(mbox,0, v) &*&  foreach(v,geq_list(phase,round));
 {
 
 open lseg(mbox,0,v);
 open foreach(v,eq_list(phase,round));
 if (v == nil) {
    close lseg(0,0,nil);
    close foreach(nil,geq_list(phase,round));
 }else{
 
 open eq_list(phase,round)(head(v));
 close geq_list(phase,round)(head(v));
 lemma_EQ_list_to_GEQ_list(mbox->next,phase,round);
 
 open lseg(mbox->next,0,tail(v));
  close lseg(mbox->next,0,tail(v));
 open foreach(tail(v),geq_list(phase,round));
 close foreach(tail(v),geq_list(phase,round));
 
 close lseg(mbox,0,cons(head(v),tail(v)));
 close foreach(cons(head(v),tail(v)),geq_list(phase,round));
 }
 }
 
 
 
@*/

// Predicate: all messages tags are EQUAL to the input tag
/*@
 predicate_ctor eq_list(int phase, int round)(struct Msg *msg) =
 msg == 0 ? true :
 msg->round |-> ?r &*& msg->phase |-> ?p &*& msg->sender |-> ?s &*& msg->response |-> ?e &*&
 msg->payload |-> ?t &*& msg->commit |-> ?a &*& malloc_block_Msg(msg)&*&
 (phase == p) &*& (round == r);
 
 predicate mbox_tag_EQ(struct List *mbox, int phase, int round) =
 lseg(mbox,0,?v) &*& foreach(v,eq_list(phase,round));
 @*/



// Predicate: all messages tags are BIGGER than the input tag
/*@
 predicate_ctor leq_list(int phase, int round)(struct Msg *msg) =
 msg == 0 ? true :
 msg->round |-> ?r &*& msg->phase |-> ?p &*& msg->sender |-> ?s &*& msg->response |-> ?e &*&
 msg->payload |-> ?t &*& msg->commit |-> ?a &*& malloc_block_Msg(msg)&*&
 (phase < p) ? true : ((phase == p) &*& (round <= r) );
 
 predicate mbox_tag_LEQ(struct List *mbox, int phase, int round) =
 lseg(mbox,0,?v) &*& foreach(v,leq_list(phase,round));
 @*/


/*@
 
 lemma void lemma_EQ_list_to_alloc_list(struct List* mbox, int phase, int round)
 requires lseg(mbox,0, ?v) &*& foreach(v,eq_list(phase,round));
 ensures lseg(mbox,0, v) &*&  foreach(v,alloc_list());
 {
 
 open lseg(mbox,0,v);
 open foreach(v,eq_list(phase,round));
 if (v == nil) {
 close lseg(0,0,nil);
 close foreach(nil,alloc_list());
 }else{
 
 open eq_list(phase,round)(head(v));
 close alloc_list()(head(v));
 lemma_EQ_list_to_alloc_list(mbox->next,phase,round);
 
 open lseg(mbox->next,0,tail(v));
 close lseg(mbox->next,0,tail(v));
 open foreach(tail(v),alloc_list());
 close foreach(tail(v),alloc_list());
 
 close lseg(mbox,0,cons(head(v),tail(v)));
 close foreach(cons(head(v),tail(v)),alloc_list());
 }
 }
 @*/

#endif /* functions_h */
