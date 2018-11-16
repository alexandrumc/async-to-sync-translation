//
//  Leader-election-Zab.c
//  
//
//  Created by Cezara on 22/10/2018.
//
#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
//#include "Zab-Discovery_Synchronization_Broadcast2.h"

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



struct arraylist;
struct arraylist *create_arraylist() ;
//@ requires true;
//@ ensures true;

/*@
 predicate arraylist(struct arraylist *a; list<void*> vs);
 @*/

/*@

predicate ltype_pred(struct Ltype *lentry)=
lentry==0 ? true :
lentry->commit |-> _ &*& lentry->op |-> _ &*& malloc_block_Ltype(lentry);

@*/

ltype *list_get(struct arraylist *a, int i);
//@ requires true;
//@ ensures ltype_pred(result);

int list_length(struct arraylist *a);
//@ requires true;
//@ ensures true;

void list_add(struct arraylist *a, void *v);
//@ requires true;
//@ ensures true;


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
    round = NewEpoch;
    
    //@ int old_epoch = epoch-1;
    //@ enum round_typ_A old_round = round;
    
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    
    
    
    while (true)
        //@ invariant ((old_epoch<epoch && round == NewEpoch));
    {
        //@close  tag_leq(old_epoch,old_round,epoch,round);
        //@assert tag_leq(old_epoch,old_round,epoch,round);
        //@open  tag_leq(old_epoch,old_round,epoch,round);
        
        round = NewEpoch;
        //@ old_epoch = epoch;
        //@ old_round = round;
        
        
        
        if(pid == leader(epoch,n)){
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->epoch = epoch;
            m->round = NewEpoch;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, to_all);
            
            free(m);
            m = NULL;
        }
        
        mbox = NULL;
        reset_timeout();
        //@close mbox_tag_geq(epoch, round,mbox);
        
        while(true)
            //@ invariant mbox_tag_geq(epoch, round,mbox);
        {
            
            m = recv();
            if (m != NULL && m->epoch >= epoch && m->round == NewEpoch){
                //@open mbox_tag_geq(epoch, round,mbox);
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                //@ close mbox_tag_geq(epoch, round,mbox);
                mbox = mbox_new;
                //@ close mbox_tag_geq(epoch, round,mbox);
                
            } else free(m);
            if (timeout()) break;
            //@ open mbox_tag_geq(epoch, round,mbox);
            if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                //@ close mbox_tag_geq(epoch, round,mbox);
                break;
            }
            //@ close mbox_tag_geq(epoch, round,mbox);
            
        }//end reception loop second round
        
        //@ assert mbox_tag_geq(epoch, round,mbox);
        //@ open  mbox_tag_geq(epoch, round,mbox);
        
        if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
            //@ old_epoch = epoch;
            epoch = mbox->message->epoch;
            
            //@ mbox_tag_geq_to_list_pred_lemma(mbox->next);
            //@ close max_tag_mbox(epoch,round,mbox->next);
            //@ close max_tag_mbox(epoch,round,mbox);
            
            //@ assert max_tag_mbox(epoch,round,mbox);
            
            //@ close tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@ open tag_leq(old_epoch,old_round,epoch,round);
            
            
            //@ max_tag_mbox_to_list_pred_lemma(mbox);
            
            list_dispose_mbox(mbox);
            mbox = NULL;
            
            //@ old_round = round;
            round = Ack_E;
            //@ close tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@ open tag_leq(old_epoch,old_round,epoch,round);
            
            
        }//end update of second round follower
        else {
            //@close mbox_tag_geq(epoch, round,mbox);
            //@ mbox_tag_geq_to_list_pred_lemma(mbox);
            list_dispose_mbox(mbox);
            mbox = NULL;
            //@ old_epoch = epoch;
            epoch++;
            //@ old_round = round;
            round = CEpoch;
            //@ close tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@ open tag_leq(old_epoch,old_round,epoch,round);
            //break; // replace by continue
            
        }//timeout go back to the loops begining into a new epoch
        
        if(round == Ack_E){
            
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->epoch = epoch;
            m->round = Ack_E;
            m->history = log;
            m->history_lenght = lastIndex;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, leader(epoch,n));
            
            free(m);
            m = NULL;
            
            if(pid == leader(epoch,n)){
                
                //@ close mbox_tag_eq(epoch, round,mbox);
                while(true)
                    //@ invariant mbox_tag_eq(epoch, round,mbox);
                {
                    
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == Ack_E){
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
                    else free(m);
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
                    log = longest_log(mbox, lastIndex);
                    
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    
                    //@ old_round = round;
                    round = New_Leader;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@ assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                }//end update third round leader
                else {
                    //@close mbox_tag_eq(epoch, round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox);
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = CEpoch;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    // break; // replace by continue
                }//timeout on loop reception go back to the loop's begining into a new epoch
            }else{
                //@ old_round = round;
                round = New_Leader;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@ assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
            }
            
            // Synchronization
            if(round == New_Leader){
                
                
                if(pid == leader(epoch,n)){
                    m = (msg *) malloc(sizeof(msg));
                    if(m==0) abort();
                    m->epoch = epoch;
                    m->round = New_Leader;
                    m->history = log;
                    m->history_lenght = lastIndex;
                    
                    //@assert(m->epoch == epoch && m->round == round);
                    send(m, to_all);
                    
                    free(m);
                    m = NULL;
                }
                
                mbox = NULL;
                reset_timeout();
                //@close mbox_tag_eq(epoch, round,mbox);
                
                while(true)
                    //@ invariant mbox_tag_eq(epoch, round,mbox);
                {
                    
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == New_Leader){
                        //@open mbox_tag_eq(epoch, round,mbox);
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
                        
                    } else free(m);
                    if (timeout()) break;
                    //@ open mbox_tag_eq(epoch, round,mbox);
                    if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        break;
                    }
                    //@ close mbox_tag_eq(epoch, round,mbox);
                    
                }//end reception fourth second round follower
                
                //@ assert mbox_tag_eq(epoch, round,mbox);
                //@ open  mbox_tag_eq(epoch, round,mbox);
                
                if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                    
                    ////@ close mbox_tag_eq(epoch,round,mbox);
                    ////@ assert mbox_tag_eq(epoch,round,mbox);
                    ////@ mbox_tag_eq_to_list_pred_lemma(mbox->next);
                    
                    lastIndex = mbox->message->history_lenght;
                    log = longest_log(mbox,lastIndex);
                    
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    
                    //@ old_round = round;
                    round = Ack_LD;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    
                }//end update of fourth round follower
                else {
                    //@close mbox_tag_eq(epoch, round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox);
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = NewEpoch;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                }//timeout from NewLeader go back to the loops begining into a new epoch
                
                if(round == Ack_LD){
                    
                    // BROADCAST
                    
                        listb *mboxB = NULL;
    			listb* mboxB_new = NULL;
    			msgb* mB = NULL;
                    
                    int len = list_length(log);
                    //@ assume (lastIndex == len-1);
                    ltype * lastEntry = list_get(log,lastIndex);
                    
                    int i = lastIndex;
                    //@ int old_phase = i -1;
                  
                    //@ open ltype_pred(lastEntry);
                    if (lastEntry!= NULL && lastEntry->commit == true) {
                        //@ old_phase = i;
                        i++;
                        lastIndex++;
                        ltype * newEntry;
                        newEntry = create_ltype(-1,false);
                        
                        list_add(log,newEntry);
                        //@ close ltype_pred(newEntry);
                        //@leak ltype_pred(newEntry);
                        
                    }
                    //@ close ltype_pred(lastEntry);
                    //@leak ltype_pred(lastEntry);
                    
                    enum round_typ_B bround = FIRST_ROUND;
                    
                    //@ enum round_typ_B old_bround = bround;
                    //@ enum round_typ_A old_round2 = round;
                    //@ int old_epoch2 = epoch;
                    while(true)
                    /*@ invariant (old_round2 == round && old_epoch2 == epoch && old_phase + 1 == i && bround == FIRST_ROUND );
                     @*/
                        
                    {
                        bround = FIRST_ROUND;
                        int leader = leader(epoch,n);
                        
                        if(pid == leader(epoch,n)) {
                            
                            mB = (msgb *) malloc(sizeof(msgb));
                            if(mB==0) abort();
                            mB->i = i;
                            mB->round = bround;
                            mB->epoch = epoch;
                            mB->lab = Ack_LD;
                            mB->sender = leader;
                            
                            
                            ltype * entry = list_get(log,lastIndex);
                            
                            //@ open ltype_pred(entry);
                            if(entry != NULL) mB->op = entry->op;
                            
                            //@ close ltype_pred(entry);
                            //@leak ltype_pred(entry);
                            
                            //@ assert (mB->lab == round && mB->epoch == epoch && mB->i == i && mB->round == bround);
                            send_msgb(mB, to_all);
                            free(mB);
                            mB = NULL;
                            
                        }
                        
                        
                        //@ close  eq_val_list_predB(epoch, round,i,bround,mboxB); 
                        while (true)
                            //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
                        {
                            mB = recv_msgb();
                            if ( mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == Ack_LD) {
                                
                                mboxB_new = (listb*) malloc(sizeof(listb));
                                if(mboxB_new==0) abort();
                                mboxB_new->message =mB;
                                if(mboxB!=0)
                                    mboxB_new->size = mboxB->size + 1;
                                else  mboxB_new->size =1 ;
                                mboxB_new->next = mboxB;
                                mboxB = mboxB_new;
                            }else free(mB);
                            
                            if (mboxB != NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader)
                                break;
                            
                            if (timeout())
                                break;
                        }
                        
                        //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                        if (mboxB!= NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader){
                            
                            
                            //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                            ltype *logi = list_get(log,i);
                            //@ open ltype_pred(logi);
                            if(logi != 0){
                                logi->op = mboxB->message->op;
                                logi->commit = false;
                                //@ close ltype_pred(logi);
                                //@leak ltype_pred(logi);
                            }
                            //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                            //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                            //@ old_bround = bround;
                            //@ old_bround = bround;
                            bround = SECOND_ROUND;
                            //@ close tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                            //@ assert tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                            //@ open tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                            
                            mB = (msgb *) malloc(sizeof(msgb));
                            if(mB==0) abort();
                            mB->i = i;
                            mB->round = bround;
                            mB->epoch = epoch;
                            mB->lab = Ack_LD;
                            mB->sender = pid;
                            
                            
                            //@ assert (mB->lab == round && mB->epoch == epoch && mB->i == i && mB->round == bround);
                            
                            send_msgb(mB, leader);
                            free(mB);
                            mB=NULL;
                            listB_dispose_no_data(mboxB);
            		    mboxB = NULL;
                            
                            if(pid==leader){
                            	//@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                                while (true)
                                    //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
                                {
                                    mB = recv_msgb();
                                    
                                    if (mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == Ack_LD) {
                                        mboxB_new = (listb*) malloc(sizeof(listb));
                                        if(mboxB_new==0) abort();
                                        mboxB_new->message = mB;
                                        if(mboxB!=0)
                                            mboxB_new->size = mboxB->size + 1;
                                        else  mboxB_new->size = 1 ;
                                        mboxB_new->next = mboxB;
                                        mboxB = mboxB_new;
                                    }else free(mB);
                                    
                                    if (timeout())
                                        break;
                                    
                                    if (mboxB != NULL && mboxB->size > n/2)
                                        break;
                                }
                                //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                                if (mboxB != NULL && mboxB->size > n/2) {
                                    //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                                    
                                    logi = list_get(log,i);
                                    //@ open ltype_pred(logi);
                                    if(logi != 0)  {
                                        logi->commit = true;
                                    }
                                    //@ close ltype_pred(logi);
                                    //@ leak ltype_pred(logi);
                                    //cmt_number ++;
                                    out(logi);
                                    
                                }
                                else {
                                    
                                    //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                                    //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                                    listB_dispose_no_data(mboxB);
                                    mboxB = NULL;
                                    
                                    break;
                                }
                            }
                            
                            //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
            		    //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                            listB_dispose_no_data(mboxB);
                            mboxB = NULL;
                            
                            //@ old_bround = bround;
                            bround = THIRD_ROUND;
                            //@ close tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                            //@ assert tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                            //@ open tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                            
                          
                            
                            
                            mB = (msgb *) malloc(sizeof(msgb));
                            if(mB==0) abort();
                            mB->i = i;
                            mB->round = bround;
                            mB->epoch = epoch;
                            mB->lab = Ack_LD;
                            mB->sender = pid;
                            
                            //@ assert (mB->lab == round && mB->epoch == epoch && mB->i == i && mB->round == bround);
                            
                            send_msgb(mB, to_all);
                            free(mB);
                            mB=NULL;
                            
                            while (true)
                                //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
                            {
                                mB = recv_msgb();
                                if (mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == Ack_LD) {
                                    
                                    mboxB_new = (listb*) malloc(sizeof(listb));
                                    if(mboxB_new==0) abort();
                                    mboxB_new->message =mB;
                                    if(mboxB!=0)
                                        mboxB_new->size = mboxB->size + 1;
                                    else  mboxB_new->size =1 ;
                                    mboxB_new->next = mboxB;
                                    mboxB = mboxB_new;
                                }else free(mB);
                                
                                if (mboxB != NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader)
                                    break;
                                
                                if (timeout())
                                    break;
                                
                            }
                            //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                            //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                            //@ open eq_val_list_predB(epoch, round,i,bround,mboxB);
                            
                            if (mboxB != NULL && mboxB->size >= 1) {
                                //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                                logi = list_get(log,i);
                                //@ open ltype_pred(logi);
                                if(logi != 0 && pid!= leader){
                                    logi->commit = true;
                                    //@ close ltype_pred(logi);
                                    //@leak ltype_pred(logi);
                                    //cmt_number++;
                                    out(logi);
                                    lastIndex++;
                                    ltype * newEntry = create_ltype(-1,false);
                                    list_add(log,newEntry);
                                    //@ close ltype_pred(newEntry);
                                    //@leak ltype_pred(newEntry);
                                    
                                    //@ old_bround = bround;
                                    bround = FIRST_ROUND;
                                    //@ old_phase = i;
                                    i++;
                                    listB_dispose_no_data(mboxB);
                                    mboxB = NULL;
                                }
                                if(pid == leader){
                                    lastIndex++;
                                    ltype * newEntry = create_ltype(in(),false);
                                    list_add(log,newEntry);
                                    //@ close ltype_pred(newEntry);
                                    //@ leak ltype_pred(newEntry);
                                }
                                
                                 //@ close ltype_pred(logi);
            	    		//@ leak ltype_pred(logi);
                                //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
            			//@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
            	
                                //@ old_bround = bround;
                                bround = FIRST_ROUND;
                                //@ old_phase = i;
                                i++;
                                listB_dispose_no_data(mboxB);
                                mboxB = NULL;
                                
                            }else{
                                //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                                //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                                listB_dispose_no_data(mboxB);
                                mboxB = NULL;
                                break;
                            }
                        } else {
                            //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                            //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                            listB_dispose_no_data(mboxB);
                            mboxB = NULL;
                            break;
                        }
                        
                        
                        
                        
                    }//end broadcast loop
                    
                    
                    //end Broadcast
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = NewEpoch;
                    
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);

                }//no Ack_LD; timeout in New_Leader; // ack_Ld Classic -> bcast
            }// no New_Leader; timeout in ACK_E;
        }//no ACK_E; timeout in NewEpoch
        //}//no NewEpoch; timeout in CEpoch;
        
    } //END LOOP LEADER_ELECTION
    //list_dispose(log);
    
    return 1;
}


/*@ predicate_ctor alloc_ctor()(struct Ltype* lentry) =
 lentry!=0 &*& lentry->commit |-> _ &*& lentry->op |-> _ &*& malloc_block_Ltype(lentry);
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
 // Lemma for converting a full max list predicate into a list predicate
 lemma void mbox_tag_eq_to_list_pred_lemma(struct List *l)
 requires mbox_tag_eq( ?val, ?val2, l);
 ensures list_pred(l);
 {
 open mbox_tag_eq(val,val2, l);
 if (l != 0) {
 
 mbox_tag_eq_to_list_pred_lemma(l->next);
 
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
 
 void list_dispose_mbox(struct List *l);
//@ requires list_pred(l);
//@ ensures emp;

void send_msgb(msgb* message, int pid);
//@ requires true;
//@ ensures true;

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

msgb* recv_msgb();
//@ requires emp;
/*@ ensures result->round |-> ?r &*& result->epoch |-> ?p &*& result->sender |-> ?s &*& result->i |-> ?e &*&
 result->op |-> ?t &*& result->lab |-> ?a &*&
    malloc_block_MsgB(result) &*& INT_MIN < p &*& p < INT_MAX;
@*/

/*@
 predicate tag_leqB(int pa1, int ra1, int pa2, int ra2, int pb1, int rb1, int pb2, int rb2) =
 (pa1 == pb1) && (ra1 = rb1) &&  ((pa2 < pb2) || (pa2==pb2 && ra2<=rb2));
 
 predicate tag_strict_leqB(int pa1, int ra1, int pa2, int ra2, int pb1, int rb1, int pb2, int rb2) =
 (pa1 == pb1) && (ra1 == rb1) &&  ((pa2 + 1 == pb2) || (pa2==pb2 && ra2<=rb2)) ;
 @*/

void listB_dispose_no_data(struct ListB *l);
//@ requires list_predB(l);
//@ ensures emp;

void out(ltype *v);
//@ requires true;
//@ ensures true;

int in();
//@ requires true;
//@ ensures true;

ltype * create_ltype(int op, bool b);
//@ requires true;
//@ ensures malloc_block_Ltype(result) &*& result!= 0 &*& result->commit |->_ &*& result->op|-> _;
