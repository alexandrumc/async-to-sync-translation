//
//  broadcast-roles-with-AUX_ROUND.c
//  
//
//  Created by C on 14/11/2018.
//
#include "stdlib.h"
#include<stdio.h>
#include<limits.h>




struct arraylist;
/*@
predicate arraylist(struct arraylist *a; list<void*> vs);
@*/

typedef struct Msg {
    int op;
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

msg* recv();
//@ requires emp;
/*@ ensures result->round |-> ?r  &*& result->sender |-> ?s &*& result->i |-> ?e &*& result->op |-> ?t &*&
    malloc_block_Msg(result) &*& INT_MIN < e &*& e < INT_MAX;
@*/
void send(msg* message, int pid);
//@ requires true;
//@ ensures true;
void list_add(struct arraylist *a, void *v);
  //@ requires arraylist(a, ?vs);
  //@ ensures arraylist(a, append(vs, cons(v, nil)));
void dispose(msg* c);
/*@
 requires c->round |-> _ &*&
 c->i |-> _ &*&
 c->op |-> _ &*& c->sender |-> _ &*& malloc_block_Msg(c);
 @*/
 //@ensures emp;

enum round_typ_B {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND} ;

// lastIndex and cmt_number are "global" variables, all processes executing broadcast have the same value.
void *list_get(struct arraylist *a, int i);
  //@ requires arraylist(a, ?vs) &*& 0 <= i &*& i < length(vs);
  //@ ensures arraylist(a, vs) &*& result == nth(i, vs);
  
ltype * create_ltype(int op, int b);
//@ requires true;
//@ ensures malloc_block_Ltype(result) &*& result!= 0 &*& result->commit |->_ &*& result->op|-> _;

void out(ltype *v);
//@ requires true;
//@ ensures true;

int list_length(struct arraylist *a);
  //@ requires arraylist(a, ?vs);
  //@ ensures arraylist(a, vs) &*& result == length(vs);

int in();
//@ requires true;
//@ ensures true;

int timeout();
//@ requires emp;
//@ ensures emp;

void list_dispose(struct List *l);
//@ requires lseg(l,0, ?v) &*& foreach(v,alloc_list());
//@ ensures emp;

int main(int argc, int pid, struct arraylist * log,  int lastIndex, int cmt_number, int leader)
//@ requires  arraylist(log, ?log_data) &*& 0<= lastIndex &*& lastIndex == length(log_data)-1 &*& foreach(log_data, alloc_ctor());
//@ ensures arraylist(log, ?nnewlog_data) &*& 0<= lastIndex &*& foreach(nnewlog_data, alloc_ctor());
{
    
    // &*& lastIndex == length(nnewlog_data)-1
    
    int n = argc;
    int to_all = n+1;
    
    
    enum round_typ_B round; //round
    int i = lastIndex; //phase
   
    //@ int old_phase = i-1;
    //@ enum round_typ_B old_round;
    
    
    struct List* mbox;
    struct List *mbox_new;
    msg *m =NULL;
    
    round = AUX_ROUND;
    
    
    
    ltype * lastEntry = list_get(log,lastIndex);
    
    //@ foreach_remove(lastEntry, log_data);
    //@ open alloc_ctor()(lastEntry);
    if (lastEntry!= NULL && lastEntry->commit == 1) {
        //@ close alloc_ctor()(lastEntry);
        //@ foreach_unremove(lastEntry, log_data);
        
        //@ old_phase = i;
        i++;
        lastIndex++;
        ltype * newEntry;
        if (pid == leader){newEntry = create_ltype(in(),0);}
        else{newEntry = create_ltype(-1,0);}
        
        list_add(log,newEntry);
        //@ close alloc_ctor()(newEntry);
        //@ close foreach(nil, alloc_ctor());
        //@ close foreach(cons(newEntry, nil), alloc_ctor());
        //@ foreach_append(log_data, cons(newEntry, nil));
    }else {
        //@ close alloc_ctor()(lastEntry);
        //@ foreach_unremove(lastEntry, log_data);
	m=NULL;
    }
    
    
    
    
    
    
    while (true)
    /*@ invariant (old_phase + 1 == i && round == AUX_ROUND ) &*&
     arraylist(log, ?newlog_data) &*&  i==lastIndex &*&
     0<= lastIndex &*& lastIndex == length(newlog_data)-1 &*&
     foreach(newlog_data, alloc_ctor());
     @*/
        
    {
        //@ assert (old_phase + 1 == i && round == AUX_ROUND );
        
        round = FIRST_ROUND;
        //@ old_round = round;
        //@ old_phase = i;
        
        //@ close tag_strict_leq(old_phase,old_round,i,round);
        //@ assert tag_strict_leq(old_phase,old_round,i,round);
        //@ open tag_strict_leq(old_phase,old_round,i,round);
        
        if (pid == leader) {
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->i = i;
            m->round = round;
            m->sender = leader;
            
            ltype * entry = list_get(log,lastIndex);
            //@ foreach_remove(entry, newlog_data);
            //@ open alloc_ctor()(entry);
            if(entry != NULL) { m->op = entry->op;}
            //@ close alloc_ctor()(entry);
            //@ foreach_unremove(entry, newlog_data);
            
            //@ assert lastIndex == length(newlog_data)-1;
            //free(entry);
            //@ assert (m->i == i && m->round == round);
            send(m, to_all);
            dispose(m);
            m = NULL;
            
            
            
            
            
            //@ old_round = round;
            round = SECOND_ROUND;
            m = (msg *) malloc(sizeof(msg));
            if(m==0){ abort();}
            m->i = i;
            m->round = round;
            m->sender = pid;

            //@ assert (m->i == i && m->round == round);
            
            send(m, leader);
            dispose(m);
            m=NULL;
            
            mbox = NULL;
            
            
            //@ close lseg(mbox,0,nil);
            //@ close foreach(nil,eq_list(i,round));
            //@ close mbox_tag_EQ(mbox,i,round);
            while (true)
                //@ invariant mbox_tag_EQ(mbox,i, round);
            {
                //@ open mbox_tag_EQ(mbox,i, round);
                m = recv();
                
                if (m!=NULL && m->i == i && m->round == round ) {
                    //@ open lseg(mbox,0,?v);
                    //@ close eq_list(i, round)(m);
                    
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) { abort();}
                    mbox_new->message =m;
                    if(mbox!=0)
                        {mbox_new->size = mbox->size + 1; }
                    else {mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    //@ close lseg(mbox,0, v);

                    mbox = mbox_new;
                    //@ close lseg(mbox,0, cons(m,v));
                    //@ close foreach(cons(m,v), eq_list(i, round));
                    
                }else { free(m);}
                
                if (timeout())
                    break;
                
                //@open lseg(mbox, 0, ?newb);
                if (mbox != NULL && mbox->size > n/2){
                    //@close  lseg(mbox, 0, newb);
                    break;
                }
                
                //@ close lseg(mbox, 0, newb);
                //@close mbox_tag_EQ(mbox,i, round);
            }
            
            //@close mbox_tag_EQ(mbox,i, round);
            
            //@ assert mbox_tag_EQ(mbox,i, round);
            //@ open mbox_tag_EQ(mbox,i, round);
            //@open lseg(mbox, 0, ?mnv);
            
            if (mbox != NULL && mbox->size > n/2) {
                //  //@ assert eq_val_list_pred(i,round,mbox);
                
                ltype *logi = list_get(log,i);
                
                //@ foreach_remove(logi, newlog_data);
                //@ open alloc_ctor()(logi);
                if(logi != 0)  {
                logi->commit = 1;
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
                // close foreach(mnv,eq_list(i,round));
                //@ lemma_EQ_list_to_alloc_list(mbox,i, old_round);
                list_dispose(mbox);
                mbox = NULL;
                break;
            }
            //@close lseg(mbox, 0, mnv);
            //@ lemma_EQ_list_to_alloc_list(mbox, i, round);
            
            //@ old_round = round;
            round = THIRD_ROUND;
            //@ close tag_strict_leq(old_phase,old_round, i,round);
            //@ assert tag_strict_leq(old_phase,old_round, i,round);
            //@ open tag_strict_leq(old_phase,old_round, i,round);
            
            
            
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) { abort(); }
            m->i = i;
            m->round = round;
            m->sender = pid;
            
            //@ assert (m->i == i && m->round == round);
            
            send(m, to_all);
            dispose(m);
            m=NULL;
            
            
            if(mbox!=NULL) { list_dispose(mbox);}
            mbox = NULL;
            
            lastIndex++;
            ltype * newEntry = create_ltype(in(),0);
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
            //@ close foreach(nil,eq_list(i,round));
            //@ close mbox_tag_EQ(mbox,i,round);
            while (true)
                //@ invariant mbox_tag_EQ(mbox, i, round);
            {
                //@ open mbox_tag_EQ(mbox,i, round);
                m = recv();
                if ( m!=NULL && m->i == i && m->round == round) {
                    
                    //@ open lseg(mbox,0,?v);
                    //@ close eq_list(i, round)(m);
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) { abort();}
                    mbox_new->message =m;
                    if(mbox!=0)
                        {mbox_new->size = mbox->size + 1;}
                    else
                    {mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    //@ close lseg(mbox,0, v);
                    mbox = mbox_new;
                    //@ close lseg(mbox,0, cons(m,v));
                    //@ close foreach(cons(m,v), eq_list(i, round));
                }else {free(m);}
                
                //@open lseg(mbox, 0, ?nv);
                //@open foreach(nv,eq_list(i, round));
                
                if (mbox != NULL && mbox->size >= 1){
                    //@open eq_list(i, round)(head(nv));
                    if(mbox->message!=NULL && mbox->message->sender == leader){
                        //@close eq_list(i, round)(head(nv));
                        //@close foreach(nv,eq_list(i, round));
                        //@close lseg(mbox, 0, nv);
                        //@close mbox_tag_EQ(mbox,i, round);
                        break;
                    }
                    //@close eq_list(i, round)(head(nv));
                }
                if (timeout()){
                    //@close foreach(nv,eq_list(i, round));
                    //@close lseg(mbox, 0, nv);
                    //@close mbox_tag_EQ(mbox,i, round);
                    break;
                }
                
                //@close foreach(nv,eq_list(i, round));
                //@close lseg(mbox, 0, nv);
                //@close mbox_tag_EQ(mbox, i, round);
            }
            
            //@ assert mbox_tag_EQ(mbox,i, round);
            
            
            //@open mbox_tag_EQ(mbox,i, round);
            //@open lseg(mbox, 0, ?nv);
            //@open foreach(nv,eq_list(i, round));
            
            
            if (mbox != NULL && mbox->size >= 1) {
                //@open eq_list(i, round)(head(nv));
                if( mbox->message!=NULL && mbox->message->sender == leader){
                    
                    if(pid != leader){
   
                        ltype *logi = list_get(log,i);
                        
                        //@ foreach_remove(logi, newlog_data);
                        //@ open alloc_ctor()(logi);
                        if(logi != 0){
                            logi->op = mbox->message->op;
                            logi->commit = 0;
                            
                        }
                        //@ close alloc_ctor()(logi);
                        //@ foreach_unremove(logi,newlog_data);
                        
                    }
                    
                    //@close eq_list(i, round)(head(nv));
                    //@close foreach(nv,eq_list(i, round));
                    //@close lseg(mbox, 0, nv);
                    //@ lemma_EQ_list_to_alloc_list(mbox,i, round);
                	list_dispose(mbox);
                	mbox = NULL;	
                }else{
                    //@close eq_list(i, round)(head(nv));
                    //@close foreach(nv,eq_list(i, round));
                    //@close lseg(mbox, 0, nv);
                    //@ lemma_EQ_list_to_alloc_list(mbox,i, round);
                    list_dispose(mbox);
                    mbox = NULL;
                    break;
                }
                
                
                
                //@ old_round = round;
                round = SECOND_ROUND;
                
                
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {abort();}
                m->i = i;
                m->round = round;
                m->sender = pid;
                
                
                //@ assert ( m->i == i && m->round == round);
                
                send(m, leader);
                dispose(m);
                m=NULL;
                
                //@ old_round = round;
                round = THIRD_ROUND;
                //@ close tag_strict_leq(old_phase,old_round,i,round);
                //@ assert tag_strict_leq(old_phase,old_round,i,round);
                //@ open tag_strict_leq(old_phase,old_round, i,round);

                //@ assert arraylist(log, ?ddata) &*& foreach(ddata, alloc_ctor());
                //@ assert lastIndex == length(ddata)-1 ;
                
                
                if(mbox!=NULL) { list_dispose(mbox); }
                mbox = NULL;
                
                //@ close lseg(mbox,0,nil);
                //@ close foreach(nil,eq_list(i,round));
                //@ close mbox_tag_EQ(mbox,i,round);
                
                while (true)
                    //@ invariant mbox_tag_EQ(mbox,i, round);
                    
                {
                    //@ open mbox_tag_EQ(mbox,i, round);
                    
                    m = recv();
                    if (m!=NULL && m->i == i && m->round == round) {
                        
                        //@ open lseg(mbox,0,?vb);
                        //@ close eq_list(i, round)(m);
                        
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) { abort();}
                        mbox_new->message =m;
                        if(mbox!=0)
                            {mbox_new->size = mbox->size + 1;}
                        else
                        { mbox_new->size =1 ;}
                        mbox_new->next = mbox;
                        //@ close lseg(mbox,0, vb);
                        
                        mbox = mbox_new;
                        //@ close lseg(mbox,0, cons(m,vb));
                        //@ close foreach(cons(m,vb), eq_list(i, round));
                        
                    }else {free(m);}
                    
                    //@open lseg(mbox, 0, ?nvb);
                    //@open foreach(nvb,eq_list(i, round));
                    
                    if (mbox != NULL && mbox->size >= 1){
                        //@open eq_list(i, round)(head(nvb));
                        if(mbox->message!=NULL && mbox->message->sender == leader){
                            //@close eq_list(i, round)(head(nvb));
                            //@close foreach(nvb,eq_list(i, round));
                            //@close lseg(mbox, 0, nvb);
                            //@close mbox_tag_EQ(mbox,i, round);
                            break;
                        }
                        //@close eq_list( i, round)(head(nvb));
                    }
                    if (timeout()){
                        //@close lseg(mbox, 0, nvb);
                        //@close foreach(nvb,eq_list(i, round));
                        //@close mbox_tag_EQ(mbox,i, round);
                        break;
                    }
                    
                    //@close foreach(nvb,eq_list(i, round));
                    //@close lseg(mbox, 0, nvb);
                    //@close mbox_tag_EQ(mbox,i, round);
                }
                
                //@ assert mbox_tag_EQ(mbox,i, round);
                //@open mbox_tag_EQ(mbox, i, round);
                //@open lseg(mbox, 0, ?nva);
                
                
                if (mbox != NULL && mbox->size >= 1) {
                    
                    /// //@ assert  mbox_tag_EQ(mbox, i, round);
                    
                    
                    
                    //@ assert arraylist(log, ?data) &*& foreach(data, alloc_ctor());
                    
                    ltype *logi = list_get(log,i);
                    
                    //@ foreach_remove(logi, data);
                    //@ open alloc_ctor()(logi);
                    if(logi != 0)
                        {
                        logi->commit = 1;
                        }
                    cmt_number++;
                    out(logi);
                    //@ close alloc_ctor()(logi);
                    //@ foreach_unremove(logi,data);
                    
                    
                    
                    lastIndex++;
                    ltype * newEntry = create_ltype(-1,0);
                    list_add(log,newEntry);
                    //@ close alloc_ctor()(newEntry);
                    //@ close foreach(nil, alloc_ctor());
                    //@ close foreach(cons(newEntry, nil), alloc_ctor());
                    //@ foreach_append(newlog_data, cons(newEntry, nil));
                    
                    
                    //@close lseg(mbox, 0, nva);
                    //@ lemma_EQ_list_to_alloc_list(mbox, i, round);
                    list_dispose(mbox);
                    mbox = NULL;
                    
                    //@ old_phase = i;
                    i++;
                    
                    //@ old_round = round;
                    round = AUX_ROUND;
                    //round = FIRST_ROUND;
                    //@ close tag_strict_leq(old_phase,old_round, i,round);
                    //@ assert tag_strict_leq(old_phase,old_round, i,round);
                    //@ open tag_strict_leq(old_phase,old_round,i,round);
                }else{
                    
                    //@close lseg(mbox, 0, nva);
                    //@ lemma_EQ_list_to_alloc_list(mbox,i, round);
                    list_dispose(mbox);
                    mbox = NULL;
                    break;
                    round = AUX_ROUND;
                }
            }else
            {
                //@close lseg(mbox, 0, ?nva);
                //@close foreach(nva,eq_list(i, round));
                //@ lemma_EQ_list_to_alloc_list(mbox,i, round);
                list_dispose(mbox);
                mbox = NULL;
                break;
                round = AUX_ROUND;
                
            }
        }
        
        
    }
    return 1;
}

/*@ predicate_ctor alloc_ctor()(struct Ltype* lentry) =
 lentry!=0 &*& lentry->commit |-> _ &*& lentry->op |-> _ &*& malloc_block_Ltype(lentry);
 
 predicate_ctor eq_list(int curr_phase, int curr_round)(struct Msg *msg) =
 msg == 0 ? true :
 msg->round |-> ?r &*&  msg->sender |-> ?s &*& msg->i |-> ?i &*&
 msg->op |-> ?op &*& malloc_block_Msg(msg)&*&
 (curr_phase == i) &*& (curr_round == r);
 
 
 predicate mbox_tag_EQ(struct List *mbox, int phase2, int round2) =
 lseg(mbox,0,?v) &*& foreach(v,eq_list(phase2,round2));
 
 
 predicate lseg(struct List* from, struct List* to, list<struct Msg *> v)
 requires from == to ? v == nil :
 from->next |-> ?next &*& from->message |-> ?msg &*& from->size |-> ?size &*&
 malloc_block_List(from) &*& lseg(next, to, ?nextv) &*& v == cons(msg,nextv);
 
 
 @*/

/*@ predicate_ctor alloc_list()(struct Msg *msg) = msg == 0 ? true : msg->round |-> ?r &*& msg->sender |-> ?s &*& msg->i |-> ?e &*& msg->op |-> ?t  &*&  malloc_block_Msg(msg);
 
 lemma void lemma_EQ_list_to_alloc_list(struct List* mbox,int phase2, int round2)
 requires lseg(mbox,0, ?v) &*& foreach(v,eq_list(phase2,round2));
 ensures lseg(mbox,0, v) &*&  foreach(v,alloc_list());
 {
 
 open lseg(mbox,0,v);
 open foreach(v,eq_list(phase2,round2));
 if (v == nil) {
 close lseg(0,0,nil);
 close foreach(nil,alloc_list());
 }else{
 
 open eq_list(phase2,round2)(head(v));
 close alloc_list()(head(v));
 lemma_EQ_list_to_alloc_list(mbox->next,phase2,round2);
 
 open lseg(mbox->next,0,tail(v));
 close lseg(mbox->next,0,tail(v));
 open foreach(tail(v),alloc_list());
 close foreach(tail(v),alloc_list());
 
 close lseg(mbox,0,cons(head(v),tail(v)));
 close foreach(cons(head(v),tail(v)),alloc_list());
 }
 }
 
 @*/

/*@
 predicate tag_leq(int pa2, int ra2, int pb2, int rb2) =
 ((pa2 < pb2) || (pa2==pb2 && ra2<=rb2));
 
 predicate tag_strict_leq(int pa2, int ra2, int pb2, int rb2) =
 ((pa2 + 1 == pb2) || (pa2==pb2 && ra2<=rb2)) ;
 @*/


