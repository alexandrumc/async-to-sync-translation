//
//  Zab-Discovery_Synchronization_Broadcast.h
//  
//
//  Created by Cezara on 22/10/2018.
//

#ifndef Leader_election_Zab_h
#define Leader_election_Zab_h
#include <stdio.h>
#include "stdlib.h"
#include <stdbool.h>
#include "broadcast-function-nested.h"


struct arraylist;

/*@
 predicate arraylist(struct arraylist *a; list<void*> vs);
 @*/

struct arraylist *create_arraylist() ;
//@ requires true;
//@ ensures true;

/*@

predicate ltype_pred(struct Ltype *lentry)=
lentry==0 ? true :
lentry->commit |-> _ &*& lentry->op |-> _ &*& malloc_block_Ltype(lentry);

@*/

struct arraylist *create_arraylist2() ;
//@ requires true;
//@ ensures true;


ltype *list_get(struct arraylist *a, int i);
//@ requires true;
//@ ensures ltype_pred(result);

int list_length(struct arraylist *a);
//@ requires true;
//@ ensures true;

void list_add(struct arraylist *a, void *v);
//@ requires true;
//@ ensures true;

void list_remove_nth(struct arraylist *a, int n);
//@ requires true;
//@ ensures true;

void list_dispose(struct arraylist* a);
//@ requires true;
//@ ensures true;

enum round_typ_A {CEpoch, NewEpoch, Ack_E, New_Leader,Ack_LD, BCAST} ;


typedef struct Msg {
    int round; //round1
    int pid;
    int epoch; //phase1
    struct arraylist *history;
    int history_lenght;
    int sender;
} msg;


/*@
 predicate tag_leq(int p1, int r1, int p2, int r2) = (p1 < p2) || (p1==p2 && r1<=r2) ;
 
 predicate tag_strict_leq(int p1, int r1, int p2, int r2) = (p1+1 == p2) || (p1==p2 && r1<=r2) ;
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
 &*& msg->round |-> ?r &*& msg->epoch |-> _ &*&
msg->pid |-> _ &*& msg->history |-> ?hs &*& msg->history_lenght |-> _ &*& msg->sender |-> _  &*& malloc_block_Msg(msg) &*& list_pred(next);
 @*/






/*@
 // Predicate describing the max of a complete linked list
 predicate max_min_list_pred(int mval, int val, int rval, struct List* n) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*&
 msg->epoch |-> ?e &*& msg->round |-> ?r &*& msg->pid |-> _ &*& msg->history |-> ?hs &*&
 msg->history_lenght |-> _ &*& msg->sender |-> _  &*& malloc_block_Msg(msg)
 &*& malloc_block_List(n)
 &*& n->next |-> ?next &*& n!= next &*& val>= e &*& mval<= e &*& rval ==r &*& n->size |-> ?size &*&
 max_min_list_pred(mval,?val1, rval, next) &*& val1 <= val;
 @*/

/*@
 // Predicate describing that all the elements of complete linked list are have geq phase than val and equal round tag with rval
 predicate mbox_tag_geq( int val, int rval,struct List* n) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?v &*& msg->round |-> ?r &*& 
msg->pid |-> _ &*& msg->history |-> _ &*& 
msg->history_lenght |-> _ &*& msg->sender |-> _  &*&
 malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val<= v &*& rval==r &*&  mbox_tag_geq(val,rval,next);
 @*/

/*@
 // Predicate describing the min of a complete linked list
 predicate mbox_tag_eq(int val, int val2, struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?v &*& msg->round |-> ?r &*& 
 msg->pid |-> _ &*& msg->history |-> ?hs &*&
 msg->history_lenght |-> _ &*& msg->sender |-> _  &*&
 malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val== v &*&  val2== r &*& mbox_tag_eq(val,val2, next) ;
 @*/


/*@
 // Predicate describing that all the elements of complete linked list are have geq phase than val and equal round tag with rval
 predicate max_tag_mbox(int val, int rval,struct List* n) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?v &*& msg->round |-> ?r &*& 
msg->pid |-> _ &*& msg->history |-> ?hs &*& 
msg->history_lenght |-> _ &*& msg->sender |-> _  &*&
 malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val>= v &*& rval==r &*&  max_tag_mbox(?val1,rval,next) &*& val1 <= val;
 @*/
 
 
 /*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void max_min_list_pred_to_max_tag_mbox_lemma(int mval, int rval, struct List *l)
 requires max_min_list_pred(mval, ?val,rval, l);
 ensures max_tag_mbox(val,rval,l);
 {
 open max_min_list_pred(mval,val,rval,l);
 if (l != 0) {
 
        max_min_list_pred_to_max_tag_mbox_lemma(mval,rval,l->next);
 
	 // We need the next two lines to let VeriFast know that l != l->next
 	open max_tag_mbox(?vval,rval,l->next);
 	close max_tag_mbox(vval,rval,l->next);
 	close max_tag_mbox(val,rval,l);
 	}
 else close max_tag_mbox(val,rval,0);
 }
 @*/
 
 
/*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void max_min_list_pred_to_list_pred_lemma(struct List *l)
 requires max_min_list_pred(?mval, ?val,?rval, l);
 ensures list_pred(l);
 {
 open max_min_list_pred(mval, val,rval,l);
 if (l != 0) {
 
    max_min_list_pred_to_list_pred_lemma(l->next);
 
	 // We need the next two lines to let VeriFast know that l != l->next
 	open list_pred(l->next);
 	close list_pred(l->next);
 	close list_pred(l);
 	}
 else close list_pred(0);
 }
 @*/
 
 /*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void max_tag_mbox_to_list_pred_lemma(struct List *l)
 requires max_tag_mbox(?val,?rval, l);
 ensures list_pred(l);
 {
 open max_tag_mbox( val,rval,l);
 if (l != 0) {
 
    max_tag_mbox_to_list_pred_lemma(l->next);
 
	 // We need the next two lines to let VeriFast know that l != l->next
 	open list_pred(l->next);
 	close list_pred(l->next);
 	close list_pred(l);
 	}
 else close list_pred(0);
 }
 @*/
 
 /*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void mbox_tag_eq_to_list_pred_lemma(struct List *l)
 requires mbox_tag_eq( ?val, ?val2, l);
 ensures list_pred(l);
 {
 open mbox_tag_eq(val,val2, l);
 if (l != 0) {
 
 mbox_tag_eq_to_list_pred_lemma(l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open list_pred(l->next);
 close list_pred(l->next);
 close list_pred(l);
 }
 else close list_pred(0);
 }
 @*/
 
 
 /*@
 // Lemma for converting a full min list predicate into a list predicate
 lemma void mbox_tag_geq_to_list_pred_lemma(struct List *l)
 requires mbox_tag_geq(?val,?rval, l);
 ensures list_pred(l);
 {
 open mbox_tag_geq(val,rval, l);
 if (l != 0) {
 mbox_tag_geq_to_list_pred_lemma(l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open list_pred(l->next);
 close list_pred(l->next);
 }
 close list_pred(l);
 }
 @*/




int max_epoch(struct List* mbox);
//@ requires mbox_tag_geq(?min,?rval,mbox) &*& mbox!=0;
//@ ensures max_min_list_pred(min,result,rval, mbox) &*& mbox!=0;
// max_lseg_pred(result,mbox,0);


void list_dispose_mbox(struct List *l);
//@ requires list_pred(l);
//@ ensures emp;

void dispose_msg(struct Msg *msg);
//@ requires msg->round |-> ?r &*& msg->epoch |-> ?v &*& msg->pid|->_ &*& msg->history|-> ?hs &*& msg->history_lenght|-> ?hslen &*& msg->sender|->_  &*&  malloc_block_Msg(msg) &*& (r==New_Leader)? arraylist(hs,?hsdata) : true;
//@ ensures emp;

bool reset_timeout();
//@ requires emp;
//@ ensures emp;

int leader(int phase, int net_size);
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;



bool timeout();
//@ requires emp;
//@ ensures emp;

msg* recv();
//@ requires emp;
//@ ensures result->round |-> ?r &*& result->epoch |-> ?v &*& result->pid|->_ &*& result->history|-> ?hs &*& result->history_lenght|-> ?hslen &*& result->sender|->_ &*& malloc_block_Msg(result) &*& INT_MIN <v &*& v < INT_MAX;

void send(msg* message, int pid);
//@ requires true;
//@ ensures true;


int max_log_size(struct List* mbox);
//@requires list_pred(mbox);
//@ensures list_pred(mbox) &*& result>=0; 

struct arraylist* longest_log(struct List* mbox, int lastIndex);
//@requires list_pred(mbox); 
//@ensures list_pred(mbox); 


void list_dispose_data(struct arraylist* a);
  //@ requires arraylist(a, ?vs)&*& foreach(vs,alloc_ctor) ;
  //@ ensures true;
  
#endif /* Leader_election_Zab_h */
