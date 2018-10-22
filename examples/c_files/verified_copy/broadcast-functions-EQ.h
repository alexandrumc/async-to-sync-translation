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

/*@
 predicate tag_leq(int pa1, int ra1, int pa2, int ra2, int pb1, int rb1, int pb2, int rb2) =
 (pa1 == pb1) && (ra1 = rb1) &&  ((pa2 < pb2) || (pa2==pb2 && ra2<=rb2));
 
 predicate tag_strict_leq(int pa1, int ra1, int pa2, int ra2, int pb1, int rb1, int pb2, int rb2) =
 (pa1 == pb1) && (ra1 == rb1) &&  ((pa2 + 1 == pb2) || (pa2==pb2 && ra2<=rb2)) ;
 @*/



/*@ predicate_ctor alloc_ctor()(struct Ltype* lentry) =
 lentry!=0 &*& lentry->commit |-> _ &*& lentry->op |-> _ &*& malloc_block_Ltype(lentry);
 
 predicate_ctor eq_list(int phase, int round, int curr_phase, int curr_round)(struct Msg *msg) =
 msg == 0 ? true :
 msg->round |-> ?r &*& msg->epoch |-> ?e &*& msg->sender |-> ?s &*& msg->i |-> ?i &*&
 msg->lab |-> ?l &*& msg->op |-> ?op &*& malloc_block_Msg(msg)&*&
 (curr_phase == i) &*& (curr_round == r) &*& round == l &*& i == curr_phase ;
 
 
 predicate mbox_tag_EQ(struct List *mbox, int phase, int round,int phase2, int round2) =
 lseg(mbox,0,?v) &*& foreach(v,eq_list(phase,round,phase2,round2));
 
 
@*/


/*@
 
 lemma void lemma_EQ_list_to_alloc_list(struct List* mbox, int phase, int round,int phase2, int round2)
 requires lseg(mbox,0, ?v) &*& foreach(v,eq_list(phase,round,phase2,round2));
 ensures lseg(mbox,0, v) &*&  foreach(v,alloc_list());
 {
 
 open lseg(mbox,0,v);
 open foreach(v,eq_list(phase,round,phase2,round2));
 if (v == nil) {
 close lseg(0,0,nil);
 close foreach(nil,alloc_list());
 }else{
 
 open eq_list(phase,round,phase2,round2)(head(v));
 close alloc_list()(head(v));
 lemma_EQ_list_to_alloc_list(mbox->next,phase,round,phase2,round2);
 
 open lseg(mbox->next,0,tail(v));
 close lseg(mbox->next,0,tail(v));
 open foreach(tail(v),alloc_list());
 close foreach(tail(v),alloc_list());
 
 close lseg(mbox,0,cons(head(v),tail(v)));
 close foreach(cons(head(v),tail(v)),alloc_list());
 }
 }
 @*/

typedef struct Msg {
    int op;
    int epoch; // phase1
    int  lab; // round1
    int i;  //phase2
    int round; // round2
    int sender;
} msg;

typedef struct Ltype {
    int op;
    bool commit;
} ltype;


typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

void out(ltype *v);
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
/*@ ensures result->round |-> ?r &*& result->epoch |-> ?p &*& result->sender |-> ?s &*& result->i |-> ?e &*&
 result->op |-> ?t &*& result->lab |-> ?a &*&
    malloc_block_Msg(result) &*& INT_MIN < p &*& p < INT_MAX;
@*/

void dispose(msg* c);
/*@
 requires c->epoch |-> _ &*& c->round |-> _ &*&
 c->i |-> _ &*& c->lab |-> _ &*&
 c->op |-> _ &*& c->sender |-> _ &*& malloc_block_Msg(c);
 @*/
 //@ensures emp;


ltype * create_ltype(int op, bool b);
//@ requires true;
//@ ensures malloc_block_Ltype(result) &*& result!= 0 &*& result->commit |->_ &*& result->op|-> _;

//
//(ltype *)malloc(sizeof(newEntry));

void list_dispose_double(struct List *l);
//@ requires lseg(l,0, ?v) &*& foreach(v,alloc_list());
//@ ensures emp;

void list_dispose_no_data(struct List *l);
//@ requires lseg(l,0, ?v) &*& foreach(v,alloc_list());
//@ ensures emp;

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
 
 /*@ predicate_ctor alloc_list()(struct Msg *msg) = msg == 0 ? true : msg->round |-> ?r &*& msg->epoch |-> ?p 
 								&*& msg->sender |-> ?s &*& msg->i |-> ?e &*&
 								msg->op |-> ?t &*& msg->lab |-> ?a &*& 
 								  malloc_block_Msg(msg);
  
  
  @*/






// Predicate: all messages tags are BIGGER than the input tag
/*@
 predicate_ctor leq_list(int phase, int round)(struct Msg *msg) =
 msg == 0 ? true :
 msg->round |-> ?r &*& msg->epoch |-> ?p &*& msg->sender |-> ?s &*& msg->i |-> ?e &*&
 msg->op |-> ?t &*& msg->lab |-> ?a &*& malloc_block_Msg(msg)&*&
 (phase < p) ? true : ((phase == p) &*& (round <= r) );
 
 predicate mbox_tag_LEQ(struct List *mbox, int phase, int round) =
 lseg(mbox,0,?v) &*& foreach(v,leq_list(phase,round));
 @*/




 
 /*@
 // Predicate describing a complete linked list
 predicate list_pred(struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next 
 &*& list_pred(next)&*& malloc_block_Msg(msg) &*&   
 msg->round |-> ?round &*& msg->lab |-> ?lab &*& msg->sender |-> ?sender &*&
  msg->epoch |-> ?e &*& msg->i |-> ?i   &*& msg->op |-> ?op;
 @*/

 







#endif /* functions_h */
