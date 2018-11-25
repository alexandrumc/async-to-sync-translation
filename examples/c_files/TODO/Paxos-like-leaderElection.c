#include "stdlib.h"
#include<stdio.h>
#include<limits.h>

typedef struct Msg {
	int round;
	int ballot;
	int leader;
} msg;


/*@
 predicate msg_pred(struct Msg * msg) =
 msg == 0? true : msg->round |-> _ &*& msg->ballot |-> _ &*& malloc_block_Msg(msg);
 @*/

enum round_typ {NewBallot_ROUND, AckBallot_ROUND, AUX_ROUND} ;

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
 &*& msg->round |-> _ &*& msg->ballot |-> _ &*& msg->leader |-> _ &*& malloc_block_Msg(msg) &*& list_pred(next);
 @*/



/*@
 // Predicate describing a linked list of size one whose only value is vbigger or equal to max
 predicate mbox_geq_1(int max, struct List* l, int len) =
 (l == 0 && len == 0) ?
 true
 :
 len == 1 &*& l->message|-> ?msg &*& l->next |-> NULL &*& l->size |-> 1 &*& malloc_block_Msg(msg) &*& msg->round |-> ?v1 &*& msg->ballot |-> ?v2 &*& msg->leader |-> ?s2  &*& malloc_block_List(l) &*& max <= v2 ?
 true : false ;
 @*/


 
 /*@
 // Predicate describing the min of a complete linked list
 predicate eq_val_list_pred(int val, int val2, struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->ballot |-> ?v &*& msg->leader |-> ?leader &*& msg->round |-> ?r &*& malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
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
 
 
int timeout();
//@ requires emp;
//@ ensures emp;

int reset_timeout();
//@ requires emp;
//@ ensures emp;

void out(int v1, int v2);
//@ requires true;
//@ ensures true;

int all_same(list *mbox, int leader);
//@ requires true;
//@ ensures true;

void send(msg* message, int pid);
//@ requires true;
//@ ensures true;

msg* recv()
//@ requires emp;
//@ ensures result->round |-> ?v1 &*& result->ballot |-> ?v &*& result->leader |-> ?leader &*& malloc_block_Msg(result) &*& INT_MIN <v &*& v < INT_MAX;
{
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) {
    abort();
    }
    //@ assume(m->ballot > INT_MIN && m->ballot < INT_MAX);
    return m;
}


void dispose(msg* c)
//@ requires c->ballot |-> _ &*& c->round |-> _ &*& c->leader |-> _ &*& malloc_block_Msg(c);
//@ ensures emp;
{
    free(c);
}


void list_dispose(struct List *l);
//@ requires list_pred(l);
//@ ensures emp;

void list_dispose1(struct List *l);
//@ requires  mbox_geq_1(?v,l,?s);
//@ ensures emp;

void dispose_list(list* c);
//@ requires  c->message |-> _ &*& c->next |-> _ &*& c->size |-> _ &*& malloc_block_List(c);
//@ ensures emp;



int coord(int net_size);
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;

int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
{
    
    int n = argc;
    int to_all = n+1;
//    int *log;
//    log = malloc(9*sizeof(int));
   
    enum round_typ round;
    int ballot;
    int pid,leader;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    ballot = 0;
    //@ int old_ballot = ballot-1;


    round = NewBallot_ROUND;
   //@ enum round_typ old_round = round;
    while(1)
        //@ invariant ((old_ballot<ballot && round == NewBallot_ROUND));
    {
        //@ old_ballot = ballot;
        round = NewBallot_ROUND;
        //@ old_round = round;
        //@ assert ((old_ballot<ballot) || (old_ballot==ballot && old_round <=round));

        if (pid == coord(n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->ballot = ballot;
            m->round = NewBallot_ROUND;
            
            //@assert(m->ballot == ballot && m->round == round);
            send(m, to_all);
            dispose(m);
            m = NULL;
        }
        reset_timeout();
        mbox = NULL;
        //@ int len = 0;
        
        while(true)
            //@ invariant mbox==NULL && len ==0;
        {
            m = recv();
            if (m != NULL && m->ballot>=ballot && m->round == NewBallot_ROUND){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else { mbox_new->size =1 ;}
                mbox_new->next = mbox;
                //@ len =1;
                mbox = mbox_new;
                //@ close mbox_geq_1(ballot,mbox,1);
            }
            else {
            	//@ close mbox_geq_1(ballot,NULL,0);
            	free(m);
            	}
            //@ open mbox_geq_1(ballot,mbox,len);
            if(mbox != NULL && mbox->size ==1 && mbox->next == NULL){
           	 //@ close mbox_geq_1(ballot, mbox,1);
           	 break;
            }
            if (timeout()){
                //@ close mbox_geq_1(ballot, mbox,0);
                break;}
        }
    
       	 //@ assert mbox_geq_1(ballot,mbox,len);
       	 //@ open mbox_geq_1(ballot,mbox,len);
         if(mbox!=NULL && mbox->size ==1 && mbox->next == NULL){
           	
           //@ close mbox_geq_1(ballot,mbox,1);
           //@ assert mbox_geq_1(ballot,mbox,1);
           //@ open mbox_geq_1(ballot,mbox,1);
           //@  old_ballot = ballot;
             ballot = mbox->message->ballot;
            //@ assert ((old_ballot<=ballot && old_round ==round));
             leader = mbox->message->leader;
            //@ close mbox_geq_1(old_ballot,mbox,1);
            
           
            //@ old_round = round;
            round = AckBallot_ROUND;
            //@ assert ((old_ballot<ballot) || (old_ballot==ballot && old_round <=round));
           
            //@ old_ballot = ballot;
            m = (msg *) malloc(sizeof(msg));
            if (m==0) {abort();}
            m->ballot = ballot;
            m->round = AckBallot_ROUND;
            m->leader = leader;
             
            //@assert(m->ballot == ballot && m->round == round);
            send(m, to_all);
            
            dispose(m);
            m = NULL;
            
            list_dispose1(mbox);
            reset_timeout();
            mbox = NULL;
            while(true)
            //@ invariant eq_val_list_pred(ballot,round,mbox);
            {
                //@ open eq_val_list_pred(ballot,round,mbox);
                m = recv();
                if (m != NULL && m->ballot == ballot && m->round == round){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {abort(); }
                    mbox_new->message =m;
                    if(mbox!=0) {mbox_new->size = mbox->size + 1;}
                    else {mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }
                else {free(m);};
                if (timeout()) {
                    break;
                }
                if(mbox != NULL && mbox->size > n/2){break;}
                
                //@ close eq_val_list_pred(ballot,round,mbox);
            }
        
            if (mbox != NULL && mbox->size > n/2){
            if (all_same(mbox,leader)==1){
                //@ assert eq_val_list_pred(ballot,round,mbox);
                // log[ballot] = leader;
                out(ballot, leader);
            }
            }
            //@ close eq_val_list_pred(ballot,round,mbox);
            //@ eq_val_list_pred_to_list_pred_lemma(mbox);
            
            if(mbox!=0) {
            list_dispose(mbox);
            }
         }else{
         	//@close mbox_geq_1(ballot,mbox,len);
             list_dispose1(mbox);
             mbox=NULL;
         }
       
        //@ old_ballot = ballot;
        ballot++;
       //@  old_round = round;
        round = NewBallot_ROUND;
        //@ assert ((old_ballot<ballot) || (old_ballot==ballot && old_round <=round));
    }
    return 1;
}
