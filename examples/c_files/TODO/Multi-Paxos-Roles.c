//
//  Leader-election-Zab.c
//  
//
//  Created by Cezara on 22/10/2018.
//
#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include "Zab-Discovery_Synchronization_Broadcast.h"
#include "broadcast-function-nested.h"

enum round_typ_A {
    CEpoch, NewEpoch, Ack_E, New_Leader, Ack_LD, BCAST
};
typedef struct Msg {
    int round;
    int pid;
    int epoch;
    struct arraylist *history;
    int history_lenght;
    int sender;
} msg;
typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;
enum round_typ_B {
    FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND
};
typedef struct MsgB {
    int op;
    int epoch;
    int lab;
    int i;
    int round;
    int sender;
} msgb;
typedef struct Ltype {
    int op;
    int commit;
} ltype;
typedef struct ListB {
    msgb *message;
    struct ListB *next;
    int size;
} listb;
struct arraylist;

struct arraylist *create_arraylist();

ltype *list_get(struct arraylist *a, int i);

int list_length(struct arraylist *a);

void list_add(struct arraylist *a, void *v);

int reset_timeout();

int coord(int net_size);

int timeout();

msg *recv();

void send(msg *message, int pid);

int max_log_size(struct List *mbox);

struct arraylist *longest_log(struct List *mbox, int lastIndex);

void list_dispose_data(struct arraylist *a);

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
    int commit = false;
    
    int i; //phase of broadcast
    enum round_typ_B  bround; // round of broadcast
    
    //@ int old_epoch = epoch-1;
    //@ enum round_typ_A old_round = round;
    
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    
    listb *mboxB = NULL;
    listb* mboxB_new = NULL;
    msgb* mB = NULL;
    
    //@ int old_phase = i-1;
    //@ int old_bround = bround;
    
    
    
    
    while (true)
        //@ invariant ((old_epoch<epoch && round == NewEpoch));
    {
        
        
        if(pid == coord(n)){
            
            //@ close  tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@open  tag_leq(old_epoch,old_round,epoch,round);
            
            round = NewEpoch;
            //@ old_epoch = epoch;
            //@ old_round = round;
            
            
            
            //@ assert round == NewEpoch;
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->epoch = epoch;
            m->round = NewEpoch;
            m->sender = pid;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, to_all);
            
            free(m);
            m = NULL;
            
            
            mbox= NULL;
            reset_timeout();
            //@close mbox_tag_geq(epoch, round,mbox);
            
            while(true)
                //@ invariant mbox_tag_geq(epoch, round,mbox);
            {
                
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewEpoch){
                    //@open mbox_tag_geq(epoch, round,mbox);
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
                    }
                    mbox_new->next = mbox;
                    //@ close mbox_tag_geq(epoch, round,mbox);
                    mbox = mbox_new;
                    //@ close mbox_tag_geq(epoch, round,mbox);
                    
                } else {
                    free(m);
                }
                if (timeout()) break;
                //@ open mbox_tag_geq(epoch, round,mbox);
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    //@ close mbox_tag_geq(epoch, round,mbox);
                    break;
                }
                //@ close mbox_tag_geq(epoch, round,mbox);
                
            }//end reception loop second round follower
            
            //@ assert mbox_tag_geq(epoch, round,mbox);
            //@ open  mbox_tag_geq(epoch, round,mbox);
            
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
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
                //tiofofof
                
                //@ old_round = round;
                round = Ack_E;
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@ assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                //@ assert round == Ack_E;
                
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                    abort();
                }
                m->epoch = epoch;
                m->round = Ack_E;
                m->history = log;
                m->history_lenght = lastIndex;
                
                //@assert(m->epoch == epoch && m->round == round);
                send(m, leader(epoch,n));
                
                free(m);
                m = NULL;
                
                list_dispose_mbox(mbox);
                mbox = NULL;
                
                //@ close mbox_tag_eq(epoch, round,mbox);
                while(true)
                    //@ invariant mbox_tag_eq(epoch, round,mbox);
                {
                    
                    m = recv();
                    if (m != NULL && m->epoch == epoch && m->round == Ack_E){
                        //@ open mbox_tag_eq(epoch, round,mbox);
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
                        }
                        mbox_new->next = mbox;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        mbox = mbox_new;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                    }
                    else {
                        free(m);
                    }
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
                    ////@ list_dispose(old_log);
                    
                    
                    round = New_Leader;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@ assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    
                    
                    
                    //@ assert round == New_Leader;
                    
                    m = (msg *) malloc(sizeof(msg));
                    if(m==0) {
                        abort();
                    }
                    m->epoch = epoch;
                    m->round = New_Leader;
                    m->history = log;
                    m->history_lenght = lastIndex;
                    
                    //@assert(m->epoch == epoch && m->round == round);
                    send(m, to_all);
                    
                    free(m);
                    m = NULL;
                    
                    
                    //@ old_round = round;
                    round = BCAST;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@ assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    
                    
                    
                    //@ assert round == BCAST;
                    m = (msg *) malloc(sizeof(msg));
                    if(m==0) {
                        abort();
                    }
                    m->epoch = epoch;
                    m->round = BCAST;
                    
                    //@assert(m->epoch == epoch && m->round == round);
                    send(m, to_all);
                    
                    free(m);
                    m = NULL;
                    
                    // out(epoch,leader(epoch,n));
                    //broadcast starts here
                    
                    
                    
                    int len = list_length(log);
                    //@ assume (lastIndex == len-1);
                    ltype * lastEntry = list_get(log,lastIndex);
                    
                    i = lastIndex;
                    //@ old_phase = i -1;
                    
                    //@ open ltype_pred(lastEntry);
                    if (lastEntry!= NULL && lastEntry->commit == true) {
                        
                        
                        //@ old_phase = i;
                        i++;
                        lastIndex++;
                        ltype * newEntry;
                        newEntry = create_ltype(in(),false);
                        list_add(log,newEntry);
                        
                        //@ close ltype_pred(newEntry);
                        //@leak ltype_pred(newEntry);
                        
                    }
                    //@ close ltype_pred(lastEntry);
                    //@leak ltype_pred(lastEntry);
                    
                    bround = FIRST_ROUND;
                    
                    //@ enum round_typ_A old_round2 = round;
                    //@ int old_epoch2 = epoch;
                    while (true)
                    /*@ invariant (old_round2 == round && old_epoch2 == epoch && old_phase + 1 == i && bround == FIRST_ROUND );
                     @*/
                    {
                        bround = FIRST_ROUND;
                        mboxB = NULL;
                        int leader = leader(epoch,n);
                        
                        mB = (msgb *) malloc(sizeof(msgb));
                        if(mB==0) {
                            abort();
                        }
                        
                        mB->i = i;
                        mB->round = bround;
                        mB->epoch = epoch;
                        mB->lab = BCAST;
                        mB->sender = leader;
                        
                        
                        ltype * entry = list_get(log,lastIndex);
                        
                        //@ open ltype_pred(entry);
                        if(entry != NULL) {
                            mB->op = entry->op;
                        }
                        
                        //@ close ltype_pred(entry);
                        //@leak ltype_pred(entry);
                        
                        //@ assert (mB->lab == round && mB->epoch == epoch && mB->i == i && mB->round == bround);
                        send_msgb(mB, to_all);
                        free(mB);
                        mB = NULL;
                        
                        mboxB = NULL;
                        
                        //@ old_bround = bround;
                        bround = SECOND_ROUND;
                        //@ close tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                        //@ assert tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                        //@ open tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                        
                        
                        mB = (msgb *) malloc(sizeof(msgb));
                        if(mB==0) {
                            abort();
                        }
                        mB->i = i;
                        mB->round = bround;
                        mB->epoch = epoch;
                        mB->lab = BCAST;
                        mB->sender = pid;
                        
                        //@ assert (mB->lab == round && mB->epoch == epoch && mB->i == i && mB->round == bround);
                        
                        send_msgb(mB, leader);
                        free(mB);
                        mB=NULL;
                        
                        while (true)
                            //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
                        {
                            mB = recv_msgb();
                            
                            if (mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                                mboxB_new = (listb*) malloc(sizeof(listb));
                                if(mboxB_new==0) {
                                    abort();
                                }
                                mboxB_new->message = mB;
                                if(mboxB!=0)
                                {
                                    mboxB_new->size = mboxB->size + 1;
                                }
                                else
                                {
                                    mboxB_new->size = 1 ;
                                }
                                    mboxB_new->next = mboxB;
                                    mboxB = mboxB_new;
                                
                            }else {
                                free(mB);
                            }
                            
                            if (timeout())
                                break;
                            
                            if (mboxB != NULL && mboxB->size > n/2)
                                break;
                        }
                        
                        //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                        if (mboxB != NULL && mboxB->size > n/2) {
                            //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                            
                            ltype *logi = list_get(log,i);
                            //@ open ltype_pred(logi);
                            if(logi != 0)  {
                                logi->commit = true;
                            }
                            //@ close ltype_pred(logi);
                            //@ leak ltype_pred(logi);
                            //cmt_number ++;
                            out_external(logi);
                            
                        }
                        else {
                            
                            //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                            //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                            listB_dispose_no_data(mboxB);
                            mboxB = NULL;
                            
                            break;
                        }
                        
                        //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                        //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                        
                        //@ old_bround = bround;
                        bround = THIRD_ROUND;
                        //@ close tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                        //@ assert tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                        //@ open tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                        
                        
                        mB = (msgb *) malloc(sizeof(msgb));
                        if(mB==0) {
                            abort();
                        }
                        mB->i = i;
                        mB->round = bround;
                        mB->epoch = epoch;
                        mB->lab = BCAST;
                        mB->sender = pid;
                        
                        //@ assert (mB->lab == round && mB->epoch == epoch && mB->i == i && mB->round == bround);
                        
                        send_msgb(mB, to_all);
                        free(mB);
                        mB=NULL;
                        
                        lastIndex++;
                        ltype * newEntry = create_ltype(in(),false);
                        list_add(log,newEntry);
                        //@ close ltype_pred(newEntry);
                        //@ leak ltype_pred(newEntry);
                        
                        
                        
                        //@ old_bround = bround;
                        bround = FIRST_ROUND;
                        //@ old_phase = i;
                        i++;
                        listB_dispose_no_data(mboxB);
                        mboxB = NULL;
                        
                        
                    }//end Broadcast loop  leader
                    
                    //Move away this it the " normal " end of a leadership when it loses quorum
                    //@ old_epoch = epoch;
                    epoch++;
                    //@ old_round = round;
                    round = NewEpoch;
                    
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    
                }//end update second round leader
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
                    // list_dispose(log); //remove when continue
                }//timeout on loop reception go back to the loop's begining into a new epoch
            }//end update of second round follower
            else {
                //@close mbox_tag_geq(epoch, round,mbox);
                //@ mbox_tag_geq_to_list_pred_lemma(mbox);
                list_dispose_mbox(mbox);
                mbox = NULL;
                //@ old_epoch = epoch;
                epoch++;
                //@ old_round = round;
                round = NewEpoch;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                //list_dispose(log); //remove when continue;
                break; // replace by continue
                
            }//timeout go back to the loops begining into a new epoch
        }//end LEADER
        else{
            //FOLLOWER
            
            //@close  tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@open  tag_leq(old_epoch,old_round,epoch,round);
            
            round = NewEpoch;
            //@ old_epoch = epoch;
            //@ old_round = round;
            
            
            
            mbox= NULL;
            reset_timeout();
            //@close mbox_tag_geq(epoch, round,mbox);
            
            while(true)
                //@ invariant mbox_tag_geq(epoch, round,mbox);
            {
                
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewEpoch){
                    //@open mbox_tag_geq(epoch, round,mbox);
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
                    }
                    mbox_new->next = mbox;
                    //@ close mbox_tag_geq(epoch, round,mbox);
                    mbox = mbox_new;
                    //@ close mbox_tag_geq(epoch, round,mbox);
                    
                } else {
                    free(m);
                }
                if (timeout()) break;
                //@ open mbox_tag_geq(epoch, round,mbox);
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    //@ close mbox_tag_geq(epoch, round,mbox);
                    break;
                }
                //@ close mbox_tag_geq(epoch, round,mbox);
                
            }//end reception loop second round follower
            
            //@ assert mbox_tag_geq(epoch, round,mbox);
            //@ open  mbox_tag_geq(epoch, round,mbox);
            
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                //@ old_epoch = epoch;
                epoch = mbox->message->epoch;
                leader = mbox->message->sender;

                //@ mbox_tag_geq_to_list_pred_lemma(mbox->next);
                //@ close max_tag_mbox(epoch,round,mbox->next);
                //@ close max_tag_mbox(epoch,round,mbox);
                
                //@ assert max_tag_mbox(epoch,round,mbox);
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                //@ max_tag_mbox_to_list_pred_lemma(mbox);
                
                
                //@ old_round = round;
                round = Ack_E;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
            
            
            
            
            //@ assert round == Ack_E;
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->epoch = epoch;
            m->round = Ack_E;
            m->history = log;
            m->history_lenght = lastIndex;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, leader(epoch,n));
            
            free(m);
            m = NULL;
            
            //@ old_round = round;
            round = New_Leader;
            //@ close tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@ open tag_leq(old_epoch,old_round,epoch,round);
            
            list_dispose_mbox(mbox);
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
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        mbox = mbox_new;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                    }
                    
                } else {
                    free(m);
                }
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
                
                //@ struct arraylist *old_log = log;
                
                //@ close mbox_tag_eq(epoch,round,mbox);
                //@ assert mbox_tag_eq(epoch,round,mbox);
                //@ mbox_tag_eq_to_list_pred_lemma(mbox->next);
                
                lastIndex = mbox->message->history_lenght;
                log = longest_log(mbox,lastIndex);
                
                
                
                //@ old_round = round;
                round = Ack_LD;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
            
            
            list_dispose_mbox(mbox);
            mbox = NULL;
            
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
                abort();
            }
            m->epoch = epoch;
            m->round = Ack_LD;
            //@assert(m->epoch == epoch && m->round == round);
            send(m, leader(epoch,n));
            free(m);
            m = NULL;
            
            //@ old_round = round;
            round = BCAST;
            //@ close tag_leq(old_epoch,old_round,epoch,round);
            //@ assert tag_leq(old_epoch,old_round,epoch,round);
            //@ open tag_leq(old_epoch,old_round,epoch,round);
            
            list_dispose_mbox(mbox);
            mbox = NULL;
            reset_timeout();
            //@close mbox_tag_eq(epoch, round,mbox);
            
            while(true)
                //@ invariant mbox_tag_eq(epoch, round,mbox);
            {
                
                m = recv();
                if (m != NULL && m->epoch == epoch && m->round == BCAST){
                    //@open mbox_tag_eq(epoch, round,mbox);
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
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        mbox = mbox_new;
                        //@ close mbox_tag_eq(epoch, round,mbox);
                    }
                    
                } else {
                    free(m);
                }
                if (timeout()) break;
                //@ open mbox_tag_eq(epoch, round,mbox);
                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    //@ close mbox_tag_eq(epoch, round,mbox);
                    break;
                }
                //@ close mbox_tag_eq(epoch, round,mbox);
                
            }//end reception loop sixth round follower
            //@ assert mbox_tag_eq(epoch, round,mbox);
            //@ open  mbox_tag_eq(epoch, round,mbox);
            
            if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                
                //@ close mbox_tag_eq(epoch,round,mbox);
                //@ assert mbox_tag_eq(epoch,round,mbox);
                //out(epoch,leader(epoch,n));
                //@ mbox_tag_eq_to_list_pred_lemma(mbox->next);
                //TODO Broadcast
                
                
                i = lastIndex;
                //@ old_phase = i -1;
                ltype * lastEntry = list_get(log,lastIndex);
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
                bround = FIRST_ROUND;
                //@ enum round_typ_A old_round2 = round;
                //@ int old_epoch2 = epoch;
                while(true)
                /*@ invariant (old_round2 == round && old_epoch2 == epoch && old_phase + 1 == i && bround == FIRST_ROUND );
                 @*/
                {
                    bround = FIRST_ROUND;
                    mboxB = NULL;
                    int leader = leader(epoch,n);
                    
                    while (true)
                        //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
                    {
                        mB = recv_msgb();
                        if ( mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                            
                            mboxB_new = (listb*) malloc(sizeof(listb));
                            if(mboxB_new==0) {
                                abort();
                            }
                            mboxB_new->message =mB;
                            if(mboxB!=0)
                            {
                                mboxB_new->size = mboxB->size + 1;
                            }
                            else
                            {
                                mboxB_new->size =1 ;
                                mboxB_new->next = mboxB;
                                mboxB = mboxB_new;
                            }
                        }else {
                            free(mB);
                        }
                        
                        if (mboxB != NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader)
                        {
                            break;
                        }
                        
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
                        
                    } else {
                        //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                        //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                        listB_dispose_no_data(mboxB);
                        mboxB = NULL;
                        break;
                    }
                    
                    
                    //@ old_bround = bround;
                    bround = SECOND_ROUND;
                    //@ close tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                    //@ assert tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                    //@ open tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                    
                    mB = (msgb *) malloc(sizeof(msgb));
                    if(mB==0) {
                        abort();
                    }
                    mB->i = i;
                    mB->round = bround;
                    mB->epoch = epoch;
                    mB->lab = BCAST;
                    mB->sender = pid;
                    
                    
                    //@ assert (mB->lab == round && mB->epoch == epoch && mB->i == i && mB->round == bround);
                    
                    send_msgb(mB, leader);
                    free(mB);
                    mB=NULL;
                    
                    //@ old_bround = bround;
                    bround = THIRD_ROUND;
                    //@ close tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                    //@ assert tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                    //@ open tag_strict_leqB(old_epoch2, old_round2,old_phase,old_bround, epoch,round, i,bround);
                    
                    listB_dispose_no_data(mboxB);
                    mboxB = NULL;
                    
                    while (true)
                        //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
                    {
                        mB = recv_msgb();
                        if (mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                            
                            mboxB_new = (listb*) malloc(sizeof(listb));
                            if(mboxB_new==0) {
                                abort();
                            }
                            mboxB_new->message =mB;
                            if(mboxB!=0)
                            {
                                mboxB_new->size = mboxB->size + 1;
                            }
                            else
                            {
                                mboxB_new->size =1 ;
                                mboxB_new->next = mboxB;
                                mboxB = mboxB_new;
                            }
                        }else {
                            free(mB);
                        }
                        
                        if (mboxB != NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader)
                        {
                            break;
                        }
                        
                        if (timeout())
                            break;
                        
                    }
                    //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                    //@ assert eq_val_list_pred(epoch, round,i,bround,mboxB);
                    //@ open eq_val_list_predB(epoch, round,i,bround,mboxB);
                    
                    if (mboxB != NULL && mboxB->size >= 1) {
                        //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
                        ltype *logi = list_get(log,i);
                        //@ open ltype_pred(logi);
                        if(logi != 0){
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
                        }
                    }else{
                        //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                        //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                        listB_dispose_no_data(mboxB);
                        mboxB = NULL;
                        break;
                    }
                    
                    //@ old_bround = bround;
                    bround = FIRST_ROUND;
                    //@ old_phase = i;
                    i++;
                    listB_dispose_no_data(mboxB);
                    mboxB = NULL;
                    
                }//end broadcast loop of follower
                
            }//end update of sixth round follower
            
            
            list_dispose_mbox(mbox);
            mbox = NULL;
            //@ old_epoch = epoch;
            epoch++;
            //@ old_round = round;
            round = NewEpoch;
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
                round = CEpoch;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                //list_dispose(log); //remove when continue;
                break; // replace by continue
                
            }//timeout go back to the loops begining into a new epoch
                
                
        }//end update of first round follower
            else {
                //@close mbox_tag_geq(epoch, round,mbox);
                //@ mbox_tag_geq_to_list_pred_lemma(mbox);
                list_dispose_mbox(mbox);
                mbox = NULL;
                //@ old_epoch = epoch;
                epoch++;
                //@ old_round = round;
                round = NewEpoch;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                //list_dispose(log); //remove when continue;
                break; // replace by continue
                
            }//timeout go back to the loops begining into a new epoch
        }//END FOLLOWER
        
    }//END LOOP LEADER_ELECTION
    
    
    //list_dispose_data(log);
    
    
    return 1;
}



