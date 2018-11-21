//
//  broadcast-roles-with-AUX_ROUND.c
//  
//
//  Created by C on 14/11/2018.
//
#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include "broadcast-functions-EQ.h"
#include "arraylist.h"






typedef struct Msg {
    int op;
    int epoch;
    int lab;
    int i;
    int round;
    int sender;
} msg;

typedef struct Ltype {
    int op;
    int commit;
} ltype;


typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

void out(ltype *v);


int in();




int timeout();

int reset_timeout();

int rand_bool();



msg* recv();

void dispose(msg* c);


ltype * create_ltype(int op, int b);

void list_dispose_double(struct List *l);

void list_dispose_no_data(struct List *l);

void dispose_list(list* c);

void send(msg* message, int pid);

int leader(int phase, int net_size);

int all_agree(struct List* l);



enum round_typ_A {PROPOSE, ACK, LEADER, BCAST} ;

enum round_typ_B {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND} ;

// lastIndex and cmt_number are "global" variables, all processes executing broadcast have the same value.


int main(int argc, int pid, struct arraylist * log,  int lastIndex, int cmt_number, int leader, int epoch)
//@ requires  arraylist(log, ?log_data) &*& 0<= lastIndex &*& lastIndex == length(log_data)-1 &*& foreach(log_data, alloc_ctor());
//@ ensures arraylist(log, ?nnewlog_data) &*& 0<= lastIndex &*& foreach(nnewlog_data, alloc_ctor());
{
    
    // &*& lastIndex == length(nnewlog_data)-1
    
    int n = argc;
    int to_all = n+1;
    
    
    enum round_typ_B round; //round
    int i = lastIndex; //phase
    int lab = BCAST; //upper_round
    // epoch upper_phase;
    
    //@ int old_phase = i-1;
    //@ int old_phase_upper = epoch;
    //@ enum round_typ_A old_round_upper = BCAST;
    //@ enum round_typ_B old_round;
    
    
    struct List* mbox;
    struct List *mbox_new;
    msg *m =NULL;
    
    round = AUX_ROUND;
    
    
    
    ltype * lastEntry = list_get(log,lastIndex);
    
    //@ foreach_remove(lastEntry, log_data);
    //@ open alloc_ctor()(lastEntry);
    if (lastEntry!= NULL && lastEntry->commit == true) {
        //@ close alloc_ctor()(lastEntry);
        //@ foreach_unremove(lastEntry, log_data);
        
        //@ old_phase = i;
        i++;
        lastIndex++;
        ltype * newEntry;
        if (pid == leader)
            
            {
            newEntry = create_ltype(in(),false);
            }
        
        else
            
            {
            newEntry = create_ltype(-1,false);
            }
        
        list_add(log,newEntry);
        //@ close alloc_ctor()(newEntry);
        //@ close foreach(nil, alloc_ctor());
        //@ close foreach(cons(newEntry, nil), alloc_ctor());
        //@ foreach_append(log_data, cons(newEntry, nil));
    }else {
        //@ close alloc_ctor()(lastEntry);
        //@ foreach_unremove(lastEntry, log_data);
        break;
    }
    
    
    
    
    
    
    while (true)
    /*@ invariant (old_round_upper == lab && old_phase_upper == epoch && old_phase + 1 == i && round == AUX_ROUND ) &*&
     arraylist(log, ?newlog_data) &*&  i==lastIndex &*&
     0<= lastIndex &*& lastIndex == length(newlog_data)-1 &*&
     foreach(newlog_data, alloc_ctor());
     @*/
        
    {
        //@ assert (old_round_upper == lab && old_phase_upper == epoch && old_phase + 1 == i && round == AUX_ROUND );
        
        round = FIRST_ROUND;
        //@ old_round = round;
        //@ old_phase = i;
        
        //@ close tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
        //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
        //@ open tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
        
        if (pid == leader) {
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->i = i;
            m->round = round;
            m->epoch = epoch;
            m->lab = BCAST;
            m->sender = leader;
            
            ltype * entry = list_get(log,lastIndex);
            //@ foreach_remove(entry, newlog_data);
            //@ open alloc_ctor()(entry);
            if(entry != NULL) {
            m->op = entry->op;
            }
            //@ close alloc_ctor()(entry);
            //@ foreach_unremove(entry, newlog_data);
            
            //@ assert lastIndex == length(newlog_data)-1;
            //free(entry);
            
            
            //@ assert (m->lab == lab && m->epoch == epoch && m->i == i && m->round == round);
            send(m, to_all);
            dispose(m);
            m = NULL;
            
            
            
            
            
            //@ old_round = round;
            round = SECOND_ROUND;
            
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0){ abort();}
            m->i = i;
            m->round = round;
            m->epoch = epoch;
            m->lab = BCAST;
            m->sender = pid;
            
            
            //@ assert (m->lab == lab && m->epoch == epoch && m->i == i && m->round == round);
            
            send(m, leader);
            dispose(m);
            m=NULL;
            
            mbox = NULL;
            
            
            //@ close lseg(mbox,0,nil);
            //@ close foreach(nil,eq_list(epoch,lab,i,round));
            //@ close mbox_tag_EQ(mbox, epoch,lab,i,round);
            while (true)
                //@ invariant mbox_tag_EQ(mbox, epoch,lab, i, round);
            {
                //@ open mbox_tag_EQ(mbox,  epoch,lab, i, round);
                m = recv();
                
                if (m!=NULL && m->i == i && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                    //@ open lseg(mbox,0,?v);
                    //@ close eq_list(epoch,lab, i, round)(m);
                    
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) { abort();}
                    mbox_new->message =m;
                    if(mbox!=0)
                        {
                        mbox_new->size = mbox->size + 1;
                        }
                    else
                    {
                    mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    //@ close lseg(mbox,0, v);
                    
                    mbox = mbox_new;
                    //@ close lseg(mbox,0, cons(m,v));
                    //@ close foreach(cons(m,v), eq_list(epoch,lab, i, round));
                    }
                }else { free(m);}
                
                if (timeout())
                    break;
                
                //@open lseg(mbox, 0, ?newb);
                if (mbox != NULL && mbox->size > n/2){
                    //@close  lseg(mbox, 0, newb);
                    break;
                }
                
                //@ close lseg(mbox, 0, newb);
                //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
            }
            
            //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
            
            //@ assert mbox_tag_EQ(mbox, epoch,lab, i, round);
            //@ open mbox_tag_EQ(mbox, epoch,lab, i, round);
            //@open lseg(mbox, 0, ?mnv);
            
            if (mbox != NULL && mbox->size > n/2) {
                //  //@ assert eq_val_list_pred(epoch, lab,i,round,mbox);
                
                ltype *logi = list_get(log,i);
                
                //@ foreach_remove(logi, newlog_data);
                //@ open alloc_ctor()(logi);
                if(logi != 0)  {
                logi->commit = true;
                }
                
                //@ close alloc_ctor()(logi);
                //@ foreach_unremove(logi,newlog_data);
                cmt_number ++;
                int size = list_length(log);
                //@ assert (size == length(newlog_data));
                //@ assert lastIndex == size-1;
                out(logi);
            }
            else {
                //@ old_round = round;
                round = AUX_ROUND;
                //@close lseg(mbox, 0, mnv);
                // close foreach(mnv,eq_list(epoch,lab,i,round));
                //@ lemma_EQ_list_to_alloc_list(mbox, epoch,lab, i, old_round);
                list_dispose_no_data(mbox);
                mbox = NULL;
                break;
            }
            //@close lseg(mbox, 0, mnv);
            //@ lemma_EQ_list_to_alloc_list(mbox, epoch,lab, i, round);
            
            //@ old_round = round;
            round = THIRD_ROUND;
            //@ close tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
            //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
            //@ open tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
            
            
            
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) { abort(); }
            m->i = i;
            m->round = round;
            m->epoch = epoch;
            m->lab = BCAST;
            m->sender = pid;
            
            //@ assert (m->lab == lab && m->epoch == epoch && m->i == i && m->round == round);
            
            send(m, to_all);
            dispose(m);
            m=NULL;
            
            
            if(mbox!=NULL) { list_dispose_no_data(mbox);}
            mbox = NULL;
            
            lastIndex++;
            ltype * newEntry = create_ltype(in(),false);
            //@close alloc_ctor()(newEntry);
            //@ close foreach(nil, alloc_ctor());
            
            list_add(log,newEntry);
            //@ close foreach(cons(newEntry, nil), alloc_ctor());
            //@ foreach_append(newlog_data, cons(newEntry,nil));
            
            //@ old_round = round;
            round = AUX_ROUND;
            //@old_phase = i;
            i++;
            
            ///END LEADER
            
        }else{
            //START FOLOWER
            mbox = NULL;
            
            //@ close lseg(mbox,0,nil);
            //@ close foreach(nil,eq_list(epoch,lab,i,round));
            //@ close mbox_tag_EQ(mbox, epoch,lab,i,round);
            while (true)
                //@ invariant mbox_tag_EQ(mbox, epoch,lab, i, round);
            {
                //@ open mbox_tag_EQ(mbox,  epoch,lab, i, round);
                m = recv();
                if ( m!=NULL && m->i == i && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                    
                    //@ open lseg(mbox,0,?v);
                    //@ close eq_list(epoch,lab, i, round)(m);
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) { abort();}
                    mbox_new->message =m;
                    if(mbox!=0)
                        {
                        mbox_new->size = mbox->size + 1;
                        }
                    else
                    {
                    mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    //@ close lseg(mbox,0, v);
                    mbox = mbox_new;
                    //@ close lseg(mbox,0, cons(m,v));
                    //@ close foreach(cons(m,v), eq_list(epoch,lab, i, round));
                    }
                }else {free(m);}
                
                //@open lseg(mbox, 0, ?nv);
                //@open foreach(nv,eq_list(epoch,lab, i, round));
                
                if (mbox != NULL && mbox->size >= 1){
                    //@open eq_list(epoch,lab, i, round)(head(nv));
                    if(mbox->message!=NULL && mbox->message->sender == leader){
                        //@close eq_list(epoch,lab, i, round)(head(nv));
                        //@close foreach(nv,eq_list(epoch,lab, i, round));
                        //@close lseg(mbox, 0, nv);
                        //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
                        break;
                    }
                    //@close eq_list(epoch,lab, i, round)(head(nv));
                }
                if (timeout()){
                    //@close foreach(nv,eq_list(epoch,lab, i, round));
                    //@close lseg(mbox, 0, nv);
                    //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
                    break;
                }
                
                //@close foreach(nv,eq_list(epoch,lab, i, round));
                //@close lseg(mbox, 0, nv);
                //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
            }
            
            //@ assert mbox_tag_EQ(mbox, epoch,lab, i, round);
            
            
            //@open mbox_tag_EQ(mbox, epoch,lab, i, round);
            //@open lseg(mbox, 0, ?nv);
            //@open foreach(nv,eq_list(epoch,lab, i, round));
            
            
            if (mbox != NULL && mbox->size >= 1) {
                //@open eq_list(epoch,lab, i, round)(head(nv));
                if( mbox->message!=NULL && mbox->message->sender == leader){
                    
                    if(pid != leader){
                        
                        
                        
                        ltype *logi = list_get(log,i);
                        
                        //@ foreach_remove(logi, newlog_data);
                        //@ open alloc_ctor()(logi);
                        if(logi != 0){
                            logi->op = mbox->message->op;
                            logi->commit = false;
                            
                        }
                        //@ close alloc_ctor()(logi);
                        //@ foreach_unremove(logi,newlog_data);
                        
                    }
                    
                    //@close eq_list(epoch,lab, i, round)(head(nv));
                    //@close foreach(nv,eq_list(epoch,lab, i, round));
                    //@close lseg(mbox, 0, nv);
                    //@ lemma_EQ_list_to_alloc_list(mbox, epoch,lab, i, round);
                }else{
                    //@close eq_list(epoch,lab, i, round)(head(nv));
                    //@close foreach(nv,eq_list(epoch,lab, i, round));
                    //@close lseg(mbox, 0, nv);
                    //@ lemma_EQ_list_to_alloc_list(mbox, epoch,lab, i, round);
                    break;
                }
                
                
                
                //@ old_round = round;
                round = SECOND_ROUND;
                
                
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {abort();}
                m->i = i;
                m->round = round;
                m->epoch = epoch;
                m->lab = BCAST;
                m->sender = pid;
                
                
                //@ assert (m->lab == lab && m->epoch == epoch && m->i == i && m->round == round);
                
                send(m, leader);
                dispose(m);
                m=NULL;
                
                
                list_dispose_no_data(mbox);
                mbox = NULL;
                
                
                //@ old_round = round;
                round = THIRD_ROUND;
                //@ close tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
                //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
                //@ open tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
                
                
                
                //@ assert arraylist(log, ?ddata) &*& foreach(ddata, alloc_ctor());
                //@ assert lastIndex == length(ddata)-1 ;
                
                
                if(mbox!=NULL) { list_dispose_no_data(mbox); }
                mbox = NULL;
                
                //@ close lseg(mbox,0,nil);
                //@ close foreach(nil,eq_list(epoch,lab,i,round));
                //@ close mbox_tag_EQ(mbox, epoch,lab,i,round);
                
                while (true)
                    //@ invariant mbox_tag_EQ(mbox, epoch,lab, i, round);
                    
                {
                    //@ open mbox_tag_EQ(mbox,  epoch,lab, i, round);
                    
                    m = recv();
                    if (m!=NULL && m->i == i && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                        
                        //@ open lseg(mbox,0,?vb);
                        //@ close eq_list(epoch,lab, i, round)(m);
                        
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) { abort();}
                        mbox_new->message =m;
                        if(mbox!=0)
                            {
                            mbox_new->size = mbox->size + 1;
                            }
                        else
                        {
                        mbox_new->size =1 ;
                        mbox_new->next = mbox;
                        //@ close lseg(mbox,0, vb);
                        
                        mbox = mbox_new;
                        //@ close lseg(mbox,0, cons(m,vb));
                        //@ close foreach(cons(m,vb), eq_list(epoch,lab, i, round));
                        }
                    }else {free(m);}
                    
                    //@open lseg(mbox, 0, ?nvb);
                    //@open foreach(nvb,eq_list(epoch,lab, i, round));
                    
                    if (mbox != NULL && mbox->size >= 1){
                        //@open eq_list(epoch,lab, i, round)(head(nvb));
                        if(mbox->message!=NULL && mbox->message->sender == leader){
                            //@close eq_list(epoch,lab, i, round)(head(nvb));
                            //@close foreach(nvb,eq_list(epoch,lab, i, round));
                            //@close lseg(mbox, 0, nvb);
                            //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
                            break;
                        }
                        //@close eq_list(epoch,lab, i, round)(head(nvb));
                    }
                    if (timeout()){
                        //@close lseg(mbox, 0, nvb);
                        //@close foreach(nvb,eq_list(epoch,lab, i, round));
                        //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
                        break;
                    }
                    
                    //@close foreach(nvb,eq_list(epoch,lab, i, round));
                    //@close lseg(mbox, 0, nvb);
                    //@close mbox_tag_EQ(mbox, epoch,lab, i, round);
                }
                
                //@ assert mbox_tag_EQ(mbox, epoch,lab, i, round);
                //@open mbox_tag_EQ(mbox, epoch,lab, i, round);
                //@open lseg(mbox, 0, ?nva);
                
                
                if (mbox != NULL && mbox->size >= 1) {
                    
                    /// //@ assert  mbox_tag_EQ(mbox, epoch,lab, i, round);
                    
                    
                    
                    //@ assert arraylist(log, ?data) &*& foreach(data, alloc_ctor());
                    
                    ltype *logi = list_get(log,i);
                    
                    //@ foreach_remove(logi, data);
                    //@ open alloc_ctor()(logi);
                    if(logi != 0)
                        {
                        logi->commit = true;
                        }
                    cmt_number++;
                    out(logi);
                    //@ close alloc_ctor()(logi);
                    //@ foreach_unremove(logi,data);
                    
                    
                    
                    lastIndex++;
                    ltype * newEntry = create_ltype(-1,false);
                    list_add(log,newEntry);
                    //@ close alloc_ctor()(newEntry);
                    //@ close foreach(nil, alloc_ctor());
                    //@ close foreach(cons(newEntry, nil), alloc_ctor());
                    //@ foreach_append(newlog_data, cons(newEntry, nil));
                    
                    
                    //@close lseg(mbox, 0, nva);
                    //@ lemma_EQ_list_to_alloc_list(mbox, epoch,lab, i, round);
                    list_dispose_no_data(mbox);
                    mbox = NULL;
                    
                    //@ old_phase = i;
                    i++;
                    
                    //@ old_round = round;
                    round = AUX_ROUND;
                    //round = FIRST_ROUND;
                    //@ close tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
                    //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
                    //@ open tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
                }else{
                    
                    //@close lseg(mbox, 0, nva);
                    //@ lemma_EQ_list_to_alloc_list(mbox, epoch,lab, i, round);
                    list_dispose_no_data(mbox);
                    mbox = NULL;
                    break;
                    round = AUX_ROUND;
                }
            }else
            {
                //@close lseg(mbox, 0, ?nva);
                //@close foreach(nva,eq_list(epoch,lab, i, round));
                //@ lemma_EQ_list_to_alloc_list(mbox, epoch,lab, i, round);
                list_dispose_no_data(mbox);
                mbox = NULL;
                break;
                round = AUX_ROUND;
                
            }
        }
        
        
    }
    return 1;
}

