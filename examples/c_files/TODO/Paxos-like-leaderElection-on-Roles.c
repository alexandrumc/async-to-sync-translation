//
//  Leader-election-Zab.c
//  
//
//  Created by Cezara on 22/10/2018.
//
#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
//#include "Leader-election-Zab.h"
//#include"Zab-Discovery_Synchronization_Broadcast2.h"

enum round_typ_A {NewBallot_ROUND, AckBallot_ROUND, AUX_ROUND} ;

struct arraylist;

/*@
 predicate arraylist(struct arraylist *a; list<void*> vs);
 @*/

struct arraylist *create_arraylist() ;
//@ requires true;
//@ ensures true;


struct arraylist *create_arraylist2() ;
//@ requires true;
//@ ensures true;


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
 // Predicate describing a linked list of size one whose only value is vbigger or equal to max
 predicate mbox_geq_1(int max, struct List* l, int len) =
 (l == 0 && len == 0) ?
 true
 :
 len == 1 &*& l->message|-> ?msg &*& l->next |-> NULL &*& l->size |-> 1 &*& malloc_block_Msg(msg) &*& msg->round |-> ?v1 &*& msg->epoch |-> ?v2 &*& msg->sender |-> ?s2 &*& msg->pid|->_ &*& msg->history|-> ?hs &*& msg->history_lenght|-> ?hslen &*& malloc_block_List(l) &*& max <= v2 ?
 true : false ;
 @*/

void list_dispose1(struct List *l);
//@ requires  mbox_geq_1(?v,l,?s);
//@ ensures emp;


int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
{
    
    int n = argc;
    int to_all = n+1;
    
    
    struct arraylist *log;
    log = create_arraylist();
    int lastIndex = list_length(log);
    
    
    enum round_typ_A round;
    int epoch;
    
    int pid;
    
    epoch = 0;
    round = NewBallot_ROUND;
    
    //@ int old_epoch = epoch-1;
    //@ enum round_typ_A old_round = round;
    
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    
    round = AUX_ROUND;
    
    while (true)
        //@ invariant ((old_epoch<epoch && round == AUX_ROUND)) ;
    {
        
        
        if(pid == leader(epoch,n)){
            
            //@ old_epoch = epoch;
            round = NewBallot_ROUND;
            
            //@ old_round = round;
            
            //@ close  tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@open  tag_leq(old_epoch,old_round,epoch,round);
            
            
            
            //@ assert round == NewBallot_ROUND;
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->epoch = epoch;
            m->round = NewBallot_ROUND;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, to_all);
            
            free(m);
            m = NULL;
            
            
            mbox= NULL;
            reset_timeout();
            
            //@ int len = 0;
            while(true)
                //@ invariant mbox==NULL && len ==0;
            {
                
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewBallot_ROUND){
                    
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) abort();
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else  mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    //@ len =1;
                    mbox = mbox_new;
                    //@ close mbox_geq_1(epoch,mbox,1);
                    
                    
                } else { //@ close mbox_geq_1(epoch,NULL,0);
                    free(m);}
                
                //@ open mbox_geq_1(epoch,mbox,len);
                
                
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    //@ close mbox_geq_1(epoch, mbox,1);
                    break;
                }
                
                if (timeout()){
                    //@ close mbox_geq_1(epoch, mbox,0);
                    break;}
                
            }//end reception loop second round follower
            
            //@ assert mbox_geq_1(epoch,mbox,len);
            //@ open mbox_geq_1(epoch,mbox,len);
            
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                //@ close mbox_geq_1(epoch,mbox,1);
                //@ assert mbox_geq_1(epoch,mbox,1);
                //@ open mbox_geq_1(epoch,mbox,1);
                //@  old_epoch = epoch;
                epoch = mbox->message->epoch;
                
                //@ close mbox_geq_1(old_epoch,mbox,1);
                // close max_tag_eq(epoch,round,mbox);
                
                // assert max_tag_mbox(epoch,round,mbox);
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                // max_tag_mbox_to_list_pred_lemma(mbox);
                //end update of second round leader
                
                
                //@ old_round = round;
                round = AckBallot_ROUND;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                m = (msg *) malloc(sizeof(msg));
                if(m==0) abort();
                m->epoch = epoch;
                m->round = AckBallot_ROUND;
                m->history = log;
                m->history_lenght = lastIndex;
                
                //@assert(m->epoch == epoch && m->round == round);
                send(m, leader(epoch,n));
                
                free(m);
                m = NULL;
                
                list_dispose1(mbox);
                mbox = NULL;
                
                //@ close mbox_tag_eq(epoch, round,mbox);
                while(true)
                    //@ invariant mbox_tag_eq(epoch, round,mbox);
                {
                    
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == AckBallot_ROUND){
                        //@ open mbox_tag_eq(epoch, round,mbox);
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) abort();
                        mbox_new->message =m;
                        if(mbox!=0)
                            mbox_new->size = mbox->size + 1;
                        else  mbox_new->size =1 ;
                        mbox_new->next = mbox;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        mbox = mbox_new;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                    }
                    else {free(m);}
                    if (timeout()){
                        break;
                    }
                    
                    //@ open mbox_tag_eq(epoch, round,mbox);
                    if(mbox != NULL && mbox->size > n/2){
                        
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        break;
                    }
                    
                    
                    //@ close mbox_tag_eq(epoch, round,mbox);
                }//end reception loop of third round:leader
                
                //@ assert mbox_tag_eq(epoch, round,mbox);
                
                //@ open  mbox_tag_eq(epoch, round,mbox);
                if(mbox != NULL && mbox->size > n/2){
                    //@ close mbox_tag_eq(epoch, round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox);
                    
                    lastIndex = max_log_size(mbox);
                    struct arraylist* old_log = log;
                    log = longest_log(mbox, lastIndex);
                    list_dispose(old_log);
                    
                    //round = New_Leader;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@ assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    //end update third round leader
                    
                    
                    //TODO Synchronization
                    // assert round == New_Leader;
                    
                    //Move away this it the " normal " end of a leadership when it loses quorum
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = AUX_ROUND;
                    
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    
                }else{
                    //@close mbox_tag_eq(epoch, round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox);
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = AUX_ROUND;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    break; // replace by continue
                }//timeout of third round go back to the loops begining with new epoch.
            } else {
                //@close mbox_geq_1(epoch,mbox,len);
                list_dispose1(mbox);
                mbox = NULL;
                //@ old_epoch = epoch;
                epoch++;
                //@ old_round = round;
                round = AUX_ROUND;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                break; // replace by continue
            }//timeout of the first round go back to the loops begining with new epoch.
            
            
        }//end LEADER
        else{
            //FOLLOWER
            
            //@close  tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@open  tag_leq(old_epoch,old_round,epoch,round);
            
            round = NewBallot_ROUND;
            //@ old_epoch = epoch;
            //@ old_round = round;
            
            
            
            
            mbox= NULL;
            reset_timeout();
            //@ int len = 0;
            while(true)
                //@ invariant mbox==NULL && len ==0;
            {
                
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewBallot_ROUND){
                    
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) abort();
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else  mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    //@ len =1;
                    mbox = mbox_new;
                    //@ close mbox_geq_1(epoch,mbox,1);
                    
                    
                } else { //@ close mbox_geq_1(epoch,NULL,0);
                    free(m);}
                
                //@ open mbox_geq_1(epoch,mbox,len);
                
                
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    //@ close mbox_geq_1(epoch, mbox,1);
                    break;
                }
                
                if (timeout()){
                    //@ close mbox_geq_1(epoch, mbox,0);
                    break;}
                
            }//end reception loop second round follower
            
            //@ assert mbox_geq_1(epoch,mbox,len);
            //@ open mbox_geq_1(epoch,mbox,len);
            
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                //@ close mbox_geq_1(epoch,mbox,1);
                //@ assert mbox_geq_1(epoch,mbox,1);
                //@ open mbox_geq_1(epoch,mbox,1);
                //@  old_epoch = epoch;
                epoch = mbox->message->epoch;
                
                //@ close mbox_geq_1(old_epoch,mbox,1);
                // close max_tag_eq(epoch,round,mbox);
                
                // assert max_tag_mbox(epoch,round,mbox);
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                //@ old_round = round;
                round = AckBallot_ROUND;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                //end update of second round follower
                
                
                
                
                //@ assert round == AckBallot_ROUND;
             
                
                m = (msg *) malloc(sizeof(msg));
                if(m==0) abort();
                m->epoch = epoch;
                m->round = AckBallot_ROUND;
                m->history = log;
                m->history_lenght = lastIndex;
                
                //@assert(m->epoch == epoch && m->round == round);
                send(m, leader(epoch,n));
                
                free(m);
                m = NULL;
                list_dispose1(mbox);
                mbox = NULL;
                
                //@ close mbox_tag_eq(epoch, round,mbox);
                while(true)
                    //@ invariant mbox_tag_eq(epoch, round,mbox);
                {
                    
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == AckBallot_ROUND){
                        //@ open mbox_tag_eq(epoch, round,mbox);
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) abort();
                        mbox_new->message =m;
                        if(mbox!=0)
                            mbox_new->size = mbox->size + 1;
                        else  mbox_new->size =1 ;
                        mbox_new->next = mbox;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        mbox = mbox_new;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                    }
                    else {free(m);}
                    if (timeout()){
                        break;
                    }
                    
                    //@ open mbox_tag_eq(epoch, round,mbox);
                    if(mbox != NULL && mbox->size > n/2){
                        
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        break;
                    }
                    
                    
                    //@ close mbox_tag_eq(epoch, round,mbox);
                }//end reception loop of third round:leader
                
                //@ assert mbox_tag_eq(epoch, round,mbox);
                
                //@ open  mbox_tag_eq(epoch, round,mbox);
                if(mbox != NULL && mbox->size > n/2){
                    //@ close mbox_tag_eq(epoch, round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox);
                    
                    lastIndex = max_log_size(mbox);
                    struct arraylist* old_log = log;
                    log = longest_log(mbox, lastIndex);
                    list_dispose(old_log);
                    
                    //round = New_Leader;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@ assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    //end update third round leader
                    
                    
                    //TODO Synchronization
                    // assert round == New_Leader;
                    
                    //Move away this it the " normal " end of a leadership when it loses quorum
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = AUX_ROUND;
                    
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    
                }else{
                    //@close mbox_tag_eq(epoch, round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox);
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = NewBallot_ROUND;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    break; // replace by continue
                }//timeout of third round go back to the loops begining with new epoch.
                
                
            } else {
                //@close mbox_geq_1(epoch,mbox,len);
                
                list_dispose1(mbox);
                mbox = NULL;
                //@ old_epoch = epoch;
                epoch++;
                //@ old_round = round;
                round = AUX_ROUND;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
            }//timeout of second round and go back to the loops begining into a new epoch
            
        }//END FOLLOWER
        
    }//END LOOP LEADER_ELECTION
    
    
    return 1;
}

int reset_timeout();
//@ requires emp;
//@ ensures emp;

int leader(int phase, int net_size);
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;



int timeout();
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

void list_dispose_mbox(struct List *l);
//@ requires list_pred(l);
//@ ensures emp;

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
 // Predicate describing a complete linked list
 predicate list_pred(struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next
 &*& msg->round |-> ?r &*& msg->epoch |-> _ &*&
 msg->pid |-> _ &*& msg->history |-> ?hs &*& msg->history_lenght |-> _ &*& msg->sender |-> _  &*& malloc_block_Msg(msg) &*& list_pred(next);
 @*/



