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
#include"Zab-Discovery_Synchronization_Broadcast2.h"


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
    round = CEpoch;
    
    
    
    //@ int old_epoch = epoch-1;
    //@ enum round_typ_A old_round = round;
    
    int i; //phase of broadcast
    enum round_typ_B  bround; // round of broadcast
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    
    listb *mboxB = NULL;
    listb* mboxB_new = NULL;
    msgb* mB = NULL;
    
    //@ int old_phase;
    //@ int old_bround;
    
    while (true)
        //@ invariant ((old_epoch<epoch && round == CEpoch)) ;
    {
        
        
        if(pid == leader(epoch,n)){
            
            //@ close  tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@open  tag_leq(old_epoch,old_round,epoch,round);
            
            round = CEpoch;
            //@ old_epoch = epoch;
            //@ old_round = round;
            
            reset_timeout();
            mbox = NULL;
            
            //@close mbox_tag_geq(epoch, round,mbox);
            
            while(true)
                //@ invariant mbox_tag_geq(epoch, round,mbox);
            {
                
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == CEpoch){
                    //@ open mbox_tag_geq(epoch, round,mbox);
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
                }
                else free(m);
                if (timeout()){
                    break;
                }
                
                //@ open mbox_tag_geq(epoch, round,mbox);
                if(mbox != NULL && mbox->size > n/2){
                    
                    //@ close mbox_tag_geq(epoch, round,mbox);
                    break;
                }
                
                
                //@ close mbox_tag_geq(epoch, round,mbox);
            }//end reception loop of first round:leader
            
            
            
            //@ assert mbox_tag_geq(epoch, round,mbox);
            
            //@ open  mbox_tag_geq(epoch, round,mbox);
            
            if(mbox != NULL && mbox->size > n/2){
                //@ old_epoch = epoch;
                //@ close mbox_tag_geq(epoch, round,mbox);
                epoch = max_epoch(mbox);
                
                
                //@open  max_min_list_pred(old_epoch,epoch,round,mbox);
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                epoch++;
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                //@close max_min_list_pred(old_epoch,epoch,round,mbox);
                //@ max_min_list_pred_to_max_tag_mbox_lemma(old_epoch,round,mbox);
                
                //@ assert max_tag_mbox(epoch, round,mbox);
                
                //@ max_tag_mbox_to_list_pred_lemma(mbox);
                
                //@ old_round = round;
                round = NewEpoch;
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                
                // end update first round leader
                
                
                
                //@ assert round == NewEpoch;
                
                m = (msg *) malloc(sizeof(msg));
                if(m==0) abort();
                m->epoch = epoch;
                m->round = NewEpoch;
                
                //@assert(m->epoch == epoch && m->round == round);
                send(m, to_all);
                
                free(m);
                m = NULL;
                
                //@ old_round = round;
                round = Ack_E;
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@ assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
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
                    struct arraylist* old_log = log;
                    log = longest_log(mbox, lastIndex);
                    list_dispose(old_log);
                    
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    
                    round = New_Leader;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@ assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
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
                    
                    //@ old_round = round;
                    round = Ack_LD;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@ assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    
                    //@ close mbox_tag_eq(epoch, round,mbox);
                    while(true)
                        //@ invariant mbox_tag_eq(epoch, round,mbox);
                    {
                        
                        m = recv();
                        if (m != NULL && m->epoch == epoch && m->round == Ack_LD){
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
                    }//end reception loop of fifth round:leader
                    
                    //@ assert mbox_tag_eq(epoch, round,mbox);
                    
                    //@ open  mbox_tag_eq(epoch, round,mbox);
                    if(mbox != NULL && mbox->size > n/2){
                        //@ close mbox_tag_eq(epoch, round,mbox);
                        //@ mbox_tag_eq_to_list_pred_lemma(mbox);
                        
                        list_dispose_mbox(mbox);
                        mbox = NULL;
                        
                        //@ old_round = round;
                        round = BCAST;
                        //@ close tag_leq(old_epoch,old_round,epoch,round);
                        //@ assert tag_leq(old_epoch,old_round,epoch,round);
                        //@ open tag_leq(old_epoch,old_round,epoch,round);
                        
                        
                        m = (msg *) malloc(sizeof(msg));
                        if(m==0) abort();
                        m->epoch = epoch;
                        m->round = BCAST;
                        
                        //@assert(m->epoch == epoch && m->round == round);
                        send(m, to_all);
                        
                        free(m);
                        m = NULL;
                        
                        //TODO Broadcast
                        
                        lastIndex = list_length(log);
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
                            if(mB==0) abort();
                            mB->i = i;
                            mB->round = bround;
                            mB->epoch = epoch;
                            mB->lab = BCAST;
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
                            
                            mboxB = NULL;
                            
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
                                
                                ltype *logi = list_get(log,i);
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
                            
                            //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                            //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                            
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

                       //END Broadcast
                        
                        //@ old_epoch = epoch;
                        epoch++;
                        //@ old_round = round;
                        round = CEpoch;
                        
                        //@ close tag_leq(old_epoch,old_round,epoch,round);
                        //@assert tag_leq(old_epoch,old_round,epoch,round);
                        //@ open tag_leq(old_epoch,old_round,epoch,round);
                        
                        list_dispose_mbox(mbox);
                        mbox = NULL;
                        
                    }//end update fifth round leader
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
                    }//timeout on loop reception go back to the loop's begining into a new epoch
                    
                    
                }else{
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
                }//timeout of third round go back to the loops begining with new epoch.
            } else {
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
            }//timeout of the second round go back to the loops begining with new epoch.
            
            
        }//end LEADER
        else{
            //FOLLOWER
            
            //@close  tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@open  tag_leq(old_epoch,old_round,epoch,round);
            
            round = CEpoch;
            //@ old_epoch = epoch;
            //@ old_round = round;
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->epoch = epoch;
            m->round = CEpoch;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, leader(epoch,n));
            
            free(m);
            m = NULL;
            
            //@ old_round = round;
            round = NewEpoch;
            
            //@ close tag_leq(old_epoch,old_round,epoch,round);
            //@assert tag_leq(old_epoch,old_round,epoch,round);
            //@ open tag_leq(old_epoch,old_round,epoch,round);
            
            
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
                
                
                //@ old_round = round;
                round = Ack_E;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
                
                
                //end update of second round follower
                
                
                
                
                //@ assert round == Ack_E;
                list_dispose_mbox(mbox);
                mbox = NULL;
                
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
                    
                    
                    log = mbox->message->history;
                    
                    lastIndex = mbox->message->history_lenght;
                    
                    
                    //@ close mbox_tag_eq(epoch,round,mbox);
                    //@ assert mbox_tag_eq(epoch,round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox->next);
                    
                    list_dispose_mbox(mbox);
                    mbox = NULL;
                    
                    //@ old_round = round;
                    round = Ack_LD;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
                    m = (msg *) malloc(sizeof(msg));
                    if(m==0) abort();
                    m->epoch = epoch;
                    m->round = Ack_LD;
                    //@assert(m->epoch == epoch && m->round == round);
                    send(m, leader(epoch,n));
                    free(m);
                    m = NULL;
                    
                    //@ old_round = round;
                    round = BCAST;
                    //@ close tag_leq(old_epoch,old_round,epoch,round);
                    //@assert tag_leq(old_epoch,old_round,epoch,round);
                    //@ open tag_leq(old_epoch,old_round,epoch,round);
                    
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
                        
                    }//end reception loop 6th round follower
                    //@ assert mbox_tag_eq(epoch, round,mbox);
                    //@ open  mbox_tag_eq(epoch, round,mbox);
                    
                    if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                        
                        //@ close mbox_tag_eq(epoch,round,mbox);
                        //@ assert mbox_tag_eq(epoch,round,mbox);
                        //	 out(epoch,leader(epoch,n));
                        //@ mbox_tag_eq_to_list_pred_lemma(mbox->next);
                        list_dispose_mbox(mbox);
                        mbox = NULL;
                        
                        //TODO Broadcast
                        
                        //Move away this it the " normal " end of a folwers when it lost leader
                        //@ old_epoch = epoch;
                        epoch++;
                        //@ old_round = round;
                        round = CEpoch;
                        //@ close tag_leq(old_epoch,old_round,epoch,round);
                        //@assert tag_leq(old_epoch,old_round,epoch,round);
                        //@ open tag_leq(old_epoch,old_round,epoch,round);
                    }//end update of sixth round follower
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
                        
                        
                    }//timeout of the 5th round: go back to the loops begining into a new epoch
                    
                    
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
                    
                }//timeout of the 4th round: go back to the loops begining into a new epoch
                
                
            } else {
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
                
                
            }//timeout of second round and go back to the loops begining into a new epoch
            
        }//END FOLLOWER
        
    }//END LOOP LEADER_ELECTION
    
    
    return 1;
}



