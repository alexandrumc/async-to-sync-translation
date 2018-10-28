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
 predicate tag_leqB(int pa1, int ra1, int pa2, int ra2, int pb1, int rb1, int pb2, int rb2) =
 (pa1 == pb1) && (ra1 = rb1) &&  ((pa2 < pb2) || (pa2==pb2 && ra2<=rb2));
 
 predicate tag_strict_leqB(int pa1, int ra1, int pa2, int ra2, int pb1, int rb1, int pb2, int rb2) =
 (pa1 == pb1) && (ra1 == rb1) &&  ((pa2 + 1 == pb2) || (pa2==pb2 && ra2<=rb2)) ;
 @*/

/*@ predicate_ctor alloc_ctor()(struct Ltype* lentry) =
 lentry!=0 &*& lentry->commit |-> _ &*& lentry->op |-> _ &*& malloc_block_Ltype(lentry);
@*/


enum round_typ_B {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND} ;

typedef struct MsgB {
    int op;
    int epoch; // phase1
    int  lab; // round1
    int i;  //phase2
    int round; // round2
    int sender;
} msgb;

typedef struct Ltype {
    int op;
    bool commit;
} ltype;


typedef struct ListB{
    msgb * message;
    struct ListB * next;
    int size;
} listb;

void out(ltype *v);
//@ requires true;
//@ ensures true;

int in();
//@ requires true;
//@ ensures true;





msgb* recv_msgb();
//@ requires emp;
/*@ ensures result->round |-> ?r &*& result->epoch |-> ?p &*& result->sender |-> ?s &*& result->i |-> ?e &*&
 result->op |-> ?t &*& result->lab |-> ?a &*&
    malloc_block_MsgB(result) &*& INT_MIN < p &*& p < INT_MAX;
@*/

void dispose(msgb* c);
/*@
 requires c->epoch |-> _ &*& c->round |-> _ &*&
 c->i |-> _ &*& c->lab |-> _ &*&
 c->op |-> _ &*& c->sender |-> _ &*& malloc_block_MsgB(c);
 @*/
 //@ensures emp;


ltype * create_ltype(int op, bool b);
//@ requires true;
//@ ensures malloc_block_Ltype(result) &*& result!= 0 &*& result->commit |->_ &*& result->op|-> _;

//
//(ltype *)malloc(sizeof(newEntry));

void listB_dispose_double(struct ListB *l);
//@ requires lsegB(l,0, ?v) &*& foreach(v,alloc_list());
//@ ensures emp;

void listB_dispose_no_data(struct ListB *l);
//@ requires list_predB(l);
//@ ensures emp;


void send_msgb(msgb* message, int pid);
//@ requires true;
//@ ensures true;

int all_agree(struct ListB* l);
//@ requires lsegB(l,0, ?v)&*& foreach(v,alloc_list());
//@ ensures lsegB(l,0, v) &*& foreach(v,alloc_list()) &*& 0 <= result &*& result<=1;



/*@
predicate listB(struct ListB* n; list<struct MsgB *> v) =
 n == 0 ? (v == nil) :
           n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_ListB(n) &*& n!=next &*& listB(next, ?vnext) &*& v == cons(msg,vnext);
 
predicate lsegB(struct ListB* from, struct ListB* to, list<struct MsgB *> v)
requires from == to ? v == nil :
                      from->next |-> ?next &*& from->message |-> ?msg &*& from->size |-> ?size &*&
                      malloc_block_ListB(from) &*& lsegB(next, to, ?nextv) &*& v == cons(msg,nextv);
 
 @*/
 
 /*@ predicate_ctor alloc_list()(struct MsgB *msg) = msg == 0 ? true : msg->round |-> ?r &*& msg->epoch |-> ?p
 								&*& msg->sender |-> ?s &*& msg->i |-> ?e &*&
 								msg->op |-> ?t &*& msg->lab |-> ?a &*& 
 								  malloc_block_MsgB(msg);
  
  
  @*/





/*@
 // Predicate describing the min of a complete linked list
 predicate eq_val_list_predB(int p1, int r1, int p2, int r2, struct ListB* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?e &*& msg->round |-> ?r  &*& msg->op |-> _ &*& msg->i |-> ?i
 &*& msg->lab |-> ?lab &*& msg->sender |-> ?s &*& 
 malloc_block_MsgB(msg) &*&
 malloc_block_ListB(n) &*& n->next |-> ?next &*& n->size |-> ?size &*&
 n!=next &*&  e == p1 &*& lab == r1 &*&  r ==r2 &*& i==p2 &*& eq_val_list_predB(p1,r1,p2,r2, next);
 @*/
 
 /*@
 // Predicate describing a complete linked list
 predicate list_predB(struct ListB* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_ListB(n) &*& n!=next
 &*& list_predB(next)&*& malloc_block_MsgB(msg) &*&
 msg->round |-> ?round &*& msg->lab |-> ?lab &*& msg->sender |-> ?sender &*&
  msg->epoch |-> ?e &*& msg->i |-> ?i   &*& msg->op |-> ?op;
 @*/

 
 /*@
 // Lemma for converting a full eq_val_ list predicate into a list predicate
 lemma void eq_val_list_pred_to_list_pred_lemmaB(struct ListB *l)
 requires eq_val_list_predB( ?p1, ?r1, ?p2, ?r2, l);
 ensures list_predB(l);
 {
 open eq_val_list_predB(p1,r1,p2,r2,l);
 if (l != 0) {
 
 eq_val_list_pred_to_list_pred_lemmaB(l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open list_predB(l->next);
 close list_predB(l->next);
 close list_predB(l);
 }
 else close list_predB(0);
 }
 @*/

/*@ lemma void foreach_remove_nth<t>(int n, list<t> xs);
requires foreach(xs, ?p) &*& 0 <= n &*& n < length(xs);
ensures foreach(remove_nth(n, xs), p) &*& p(nth(n, xs));
@*/




#endif /* functions_h */
