#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include "broadcast-function-nested.h"
#include "Zab-Discovery_Synchronization_Broadcast.h"




enum round_typ_A {PROPOSE, ACK, LEADER, BCAST} ;

enum round_typ_B {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND} ;

 // lastIndex and cmt_number are "global" variables, all processes executing broadcast have the same value.


void broadcast(int argc, int pid, struct arraylist * log,  int lastIndex, int cmt_number, int leader, int epoch)
//@ requires  true;
//@ ensures true;
{

    // &*& lastIndex == length(nnewlog_data)-1
    
    int n = argc;
    int to_all = n+1;
    
    
    enum round_typ_B bround; //round
    int i = lastIndex; //phase
    enum round_typ_A round = BCAST; //upper_round
    // epoch upper_phase;
    
    //@ int old_phase = i-1;
    //@ int old_epoch = epoch;
    //@ enum round_typ_A old_round = BCAST;
    //@ enum round_typ_B old_bround;
    
    
    struct ListB* mboxB;
    struct ListB *mboxB_new;
    msgb *mB =NULL;
    
    bround = FIRST_ROUND;
    
    
   
        ltype * lastEntry = list_get(log,lastIndex);
        
        //@ open ltype_pred(lastEntry);
        if (lastEntry!= NULL && lastEntry->commit == true) {
            
            
            //@ old_phase = i;
            i++;
            lastIndex++;
            ltype * newEntry;
            if (pid == leader) 
            	 
                 newEntry = create_ltype(in(),false);
                 
            else 
        	 
                newEntry = create_ltype(-1,false);
             
            list_add(log,newEntry);
            //@ close ltype_pred(newEntry);
            //@leak ltype_pred(newEntry);
            
        }
        //@ close ltype_pred(lastEntry);
        //@leak ltype_pred(lastEntry);
    
    
    
    
    
    while (true)
    /*@ invariant (old_round == round && old_epoch == epoch && old_phase + 1 == i && bround == FIRST_ROUND );
       @*/ 
       
    {
        //@ assert (old_round == round && old_epoch == epoch && old_phase + 1 == i && bround == FIRST_ROUND );
        
        bround = FIRST_ROUND;
        //@ old_bround = bround;
        //@ old_phase = i;
        
        //@ close tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
        //@ assert tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
        //@ open tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
        
        if (pid == leader) {
            
            mB = (msgb *)malloc(sizeof(msgb));
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
        }
        
        mboxB = NULL;
         
        
        while (true)
        
       //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
            
        {
            mB = recv_msgb();
            if ( mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                
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
        
        
        if (mboxB != NULL && mboxB->size >= 1 && mboxB->message!=NULL && mboxB->message->sender == leader){
            
            
            //@ assert eq_val_list_predB(epoch, round,i,bround,mboxB);
            if(pid != leader){
            
                
                
                 ltype *logi = list_get(log,i);
                   
                 //@ open ltype_pred(logi);  
                 if(logi != 0){
                        logi->op = mboxB->message->op;
                        logi->commit = false;
                        
                    }
                    //@ close ltype_pred(logi);
            	    //@leak ltype_pred(logi);
                    
             
                       
            }
            
            // check from here on
            
            //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
            //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
            //@ old_bround = bround;
            bround = SECOND_ROUND;
            
            
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

            
            listB_dispose_no_data(mboxB);
            mboxB = NULL;
            
            if (pid == leader) {
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
                        else  mboxB_new->size =1 ;
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
                    cmt_number ++;
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
                //@ close tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
                //@ assert tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
                //@ open tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
            }
            else {
                //@ old_bround = bround;
                bround = THIRD_ROUND;
                //@ close tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
                //@ assert tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
                //@ open tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
            }
            
           
            //thirds round starts  here
            if (pid == leader) {
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
            }
            
            
            listB_dispose_no_data(mboxB);
            mboxB = NULL;
            
            while (true) 
            //@ invariant eq_val_list_predB(epoch, round,i,bround,mboxB);
            {
                mB = recv_msgb();
                if (mB!=NULL && mB->i == i && mB->epoch == epoch && mB->round == bround && mB->lab == BCAST) {
                    
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
                
                if(pid != leader){
                      	ltype *logi = list_get(log,i);
                	//@ open ltype_pred(logi);
                	if(logi != 0){
                    		logi->commit = true;
                    		
            		}
            		//@ close ltype_pred(logi);
            		//@leak ltype_pred(logi);
                	cmt_number++;
                	
                	out(logi);
                	
                	
                }
                if(pid == leader){
                    
                    lastIndex++;
                    ltype * newEntry = create_ltype(in(),false);
                    list_add(log,newEntry);
                    //@ close ltype_pred(newEntry);
            	    //@ leak ltype_pred(newEntry);         
                    
                }else {
                	lastIndex++;
                 	ltype * newEntry = create_ltype(-1,false);
                 	list_add(log,newEntry);
                 	//@ close ltype_pred(newEntry);
           		//@leak ltype_pred(newEntry);
          		
            	}
                
                //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                listB_dispose_no_data(mboxB);
                mboxB = NULL;
                
                //@ old_phase = i;
                i++;
                
                //@ old_bround = bround;
                //bround = AUX_ROUND;
                bround = FIRST_ROUND;
                //@ close tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
                //@ assert tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
                //@ open tag_strict_leqB(old_epoch, old_round,old_phase,old_bround, epoch,round, i,bround);
            }else
            {
            
          
                //@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                listB_dispose_no_data(mboxB);
                mboxB = NULL;
                break;
            
            }
        }else{
        	//@ close eq_val_list_predB(epoch, round,i,bround,mboxB);
                //@ eq_val_list_pred_to_list_pred_lemmaB(mboxB);
                listB_dispose_no_data(mboxB);
                mboxB = NULL;
                break;}
        
    }
}
