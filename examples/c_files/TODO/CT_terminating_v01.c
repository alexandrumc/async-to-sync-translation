#include "stdlib.h"
#include<stdio.h>
#include<limits.h>

//
//  CT_terminating_v01.c
//  
//
//  Created by Cezara on 14/10/2018.
//

#include "CT_terminating_v01.h"


int main(int argc, char**argv) //@ : main
//@ requires true;
//@ ensures true;
{
    int state = 0;
    
    int round = 0;
    //@ int old_round = round;
    
    int myid = 0;
    int timestamp = 0;
    
    int phase = 0;
    //@ int old_phase = -1;
    
    int estimate = in();
    int leader = 0;
    
    int n = argc;
    int to_all = n+1;
    
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    
    int ack = 0;
    
    msg* m = NULL;
    round = FIRST_ROUND;
    phase = 1;
    
    ack = rand_bool();
    
    while(1)
    //@ invariant old_phase < phase && round == FIRST_ROUND;
    {
        
        round = FIRST_ROUND;
        //@ old_phase = phase;
        //@ old_round = round;
        
        leader = leader(phase,n);
        
        m = NULL;
        m = (msg *) malloc(sizeof(msg));
        if (m == 0) abort();
        
        m->phase = phase;
        m->round = round;
        m->estimate = estimate;
        m->sender = myid;
        m->timestamp = timestamp;
        m->ack = 0;
        
        //@assert(m->phase == phase && m->round == round);
        send(m, leader);
        
        dispose(m);
        m = NULL;
        mbox = NULL;


        
        if (myid == leader) {
            //@ close list_pred(mbox);
            while (true)
                //@ invariant eq_val_list_pred(phase,round,mbox);
                {
                    //@open eq_val_list_pred(phase,round,mbox);
                    m = recv();
        
                    if (m!= NULL && m->round == FIRST_ROUND && m->phase == phase) {
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) abort();
                        mbox_new->message =m;
        
                        if(mbox!=0)
                        {
                            mbox_new->size = mbox->size + 1;
                            }
                        else  mbox_new->size =1 ;
                        mbox_new->next = mbox;
                        mbox = mbox_new;
                        if (mbox->size >= (n + 1) / 2) {
                            //@ close eq_val_list_pred(phase,round,mbox);
                                break;
                            }
                        }else if (m!=NULL && m->round == FOURTH_ROUND ){
                    	
                            mbox_new = (list*) malloc(sizeof(list));
                            if(mbox_new==0) abort();
                            mbox_new->message = m;
                            if(mbox!=0)
                                mbox_new->size = mbox->size + 1;
                            else  mbox_new->size =1 ;
                            mbox_new->next = mbox;
                         
                            mbox = mbox_new;
                                  	
                            break;
                        } else free(m);
                    }
                //@ close mbox_CT_mbox(mbox, phase, round);
                //@ open list_pred(NULL);
                //@ assert mbox_CT_mbox(mbox, phase, round);
            
            //@ open mbox_CT_mbox(mbox, phase, round);
            
            if(mbox!=0 && mbox->message!=NULL){
            	if (mbox->message->round == FOURTH_ROUND){
                    //@old_round = round;
                    round = FOURTH_ROUND;
                    
                    //@ close  max_tag_of_mbox(mbox, phase, round);
                    //@ assert max_tag_of_mbox(mbox, phase, round);
                    //@ open max_tag_of_mbox(mbox, phase, round);
                
                    estimate = mbox->message->estimate;
                    state = 1;
                
                    //@ close mbox_CT_mbox(mbox, phase, old_round);
                    //@ leak mbox_CT_mbox(mbox, phase, old_round);
                
                    list_dispose(mbox);
                    mbox = NULL;
                
                    break;
          	  }else{
               		 if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                    
                    //@ assert eq_val_list_pred( phase, round,mbox);
                   	 m = max_timestamp(mbox);
                   	 estimate = m->estimate;
                   	 free(m);
                   	 m=NULL;
                    
                	}
            	}
            }
            //@ close mbox_CT_mbox(mbox, phase, round);
           //@leak mbox_CT_mbox(mbox, phase, round);
            list_dispose(mbox);
            mbox = NULL;
    }
        
        //@ old_round = round;
        round = SECOND_ROUND;
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open  tag_strict_leq(old_phase,old_round,phase,round);
        
        
        if (myid == leader) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            
             //@assert(m->phase == phase && m->round == round);
            send(m,to_all);
            dispose(m);
            m = NULL;
        }
        //@ close mbox_CT_mbox(mbox, phase, round);
        //@leak mbox_CT_mbox(mbox,phase,round);
        
        list_dispose(mbox);
        mbox = NULL;
        
        //@ close list_pred(mbox);
        while (1) 
        //@ invariant eq_val_list_pred(phase,round,mbox);
        {
            //@open eq_val_list_pred(phase,round,mbox);
            m = recv();
            if (m!= NULL && m->phase == phase && m->round == SECOND_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
                 
            } else {
                if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size = 1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
                break;
                } else free(m);
            }
            if (timeout()) break;
            else if (mbox!=0 && mbox->message->sender ==leader){
            	//@ close eq_val_list_pred(phase,round,mbox);
                break;
                }
        }
        
        //@ close mbox_CT_mbox(mbox, phase, round);
        //@ open list_pred(NULL);
        //@ assert mbox_CT_mbox(mbox, phase, round);
            
       //@ open mbox_CT_mbox(mbox, phase, round);
        
        
        if(mbox!=0 && mbox->message!=NULL){
            
            if( mbox->message->round == FOURTH_ROUND){
            	//@ old_round =  round; 
            	round = FOURTH_ROUND;
            	//@ close max_tag_of_mbox(mbox, phase, round);
            	//@ assert max_tag_of_mbox(mbox, phase, round);
            	//@ open max_tag_of_mbox(mbox, phase, round);
            	estimate = mbox->message->estimate;
            	state = 1;
                
                 //@ close mbox_CT_mbox(mbox, phase, old_round);
            	 //@ leak mbox_CT_mbox(mbox, phase, old_round);
                
                  list_dispose(mbox);
                  mbox = NULL;
                    
            	break;
            }else{
                if (mbox!=0) {
                    //@ assert eq_val_list_pred( phase, round,mbox);
                    estimate = mbox->message->estimate;
                    timestamp = phase;
                }
            }
        }
        //@ close mbox_CT_mbox(mbox, phase, round);
        //@ leak mbox_CT_mbox(mbox, phase, round);
        list_dispose(mbox);
        mbox = NULL;
            
        
        
       //@ old_round = round;
        round = THIRD_ROUND;
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);
        
        if (timestamp == phase) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->timestamp = timestamp;
            
             //@assert(m->phase == phase && m->round == round);
            send(m,leader);
            dispose(m);
            m = NULL;
        }
        
         
        if (myid == leader) {

            mbox = NULL;
            //@ close list_pred(mbox);
            
            while (1) 
            //@ invariant eq_val_list_pred(phase,round,mbox);
            {
            	//@open eq_val_list_pred(phase,round,mbox);
                m = recv();
                if (m!= NULL && m->round == THIRD_ROUND && m->phase == phase) {
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) abort();
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else  mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                    
                    if (mbox->size > n/2) 
                    {
                    	//@ close eq_val_list_pred(phase,round,mbox);
                    	break;
                    }
                    
                }else  if (m!= NULL && m->round == FOURTH_ROUND) {
                            mbox_new = (list*) malloc(sizeof(list));
                            if(mbox_new==0) abort();
                            mbox_new->message =m;
                            if(mbox!=0)
                                mbox_new->size = mbox->size + 1;
                            else  mbox_new->size =1 ;
                            mbox_new->next = mbox;
                            mbox = mbox_new;
                            break;
                        }else {free(m);}
                
                if (timeout()) break;
            }
            //@ close mbox_CT_mbox(mbox, phase, round);
            //@ open list_pred(NULL);
            //@ assert mbox_CT_mbox(mbox, phase, round);
        

            
            //@ open mbox_CT_mbox(mbox, phase, round);
            
            if(mbox!=0 && mbox->message!=NULL){
            	if (mbox->message->round == FOURTH_ROUND){
                    //@old_round = round;
                    round = FOURTH_ROUND;
                    
                    //@ close  max_tag_of_mbox(mbox, phase, round);
                    //@ assert max_tag_of_mbox(mbox, phase, round);
                    //@ open max_tag_of_mbox(mbox, phase, round);
                
                    estimate = mbox->message->estimate;
                    state = 1;
                
                    //@ close mbox_CT_mbox(mbox, phase, old_round);
                    //@ leak mbox_CT_mbox(mbox, phase, old_round);
                
                    list_dispose(mbox);
                    mbox = NULL;
                
                    break;
          	  }else{
                	if (mbox!=0 && mbox->size >= (n + 1) / 2) {
                        	//@ assert eq_val_list_pred( phase, round,mbox);
                        	ack =1;
                		}
                	}
            	}
               //@ close mbox_CT_mbox(mbox, phase, round);
            	//@leak mbox_CT_mbox(mbox, phase, round);
           	 list_dispose(mbox);
            	mbox = NULL;
        }
            
        
        
        
        //@ old_phase = phase;
        //@ old_round = round;
        round = FOURTH_ROUND;
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);
        
        
        if (myid == leader && ack == 1) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->sender = myid;
            m->phase = phase;
            m->round = round;
            m->estimate = estimate;
            m->ack = ack;
            
            //@assert(m->phase == phase && m->round == round);
            send(m,to_all);
            dispose(m);
            m = NULL;
            
        }
        
        //@close mbox_CT_mbox(mbox,phase,round);
        //@leak mbox_CT_mbox(mbox,phase,round);
        list_dispose(mbox);
        mbox = NULL;
        
        while (1)
        //@ invariant eq_max_val_list_pred(phase,round,mbox);
        {
            //@open eq_max_val_list_pred(phase,round,mbox);
            m = recv();
            if (m!= NULL && m->round == FOURTH_ROUND) {
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
                break;
            }else{free(m);}
            
            if (timeout()) break;
        }
        ////@ assert mbox_tag_geq(mbox, phase, round);
        //@  assert  eq_max_val_list_pred(phase,round,mbox);
        //@ close eq_max_val_list_pred(phase,round,mbox);
        
        
        
        if (mbox!=0)
        {
            //@ open eq_max_val_list_pred(phase,round,mbox);
            //@ assert eq_max_val_list_pred(phase,round,mbox);
            //@ close eq_max_val_list_pred(phase,round,mbox);
            
            ////@ open eq_max_val_list_predmax_tag_of_mbox(mbox, phase, round);
            ////@ assert max_tag_of_mbox(mbox, phase, round);
            ////@ close max_tag_of_mbox(mbox, phase, round);
            estimate = mbox->message->estimate;
            state = 1;
            out(myid,estimate);
            
            //@ close eq_max_val_list_pred(phase,round,mbox);
 	        //@ leak eq_max_val_list_pred(phase,round,mbox);
            list_dispose(mbox);
            mbox = NULL;
            break;
            
        }
        
        
        
        phase = phase + 1;
        //@old_round = round;
        round = FIRST_ROUND;
        
        
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);
        
    }
    
   

    return 1;
}
	
