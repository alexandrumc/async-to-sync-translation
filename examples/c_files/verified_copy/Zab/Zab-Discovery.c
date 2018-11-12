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
    
    
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    
  
    
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
                
                
                
            }// end update first round leader
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
                break; // replace by continue 
            }//timeout go back to the loops begining with new epoch.
            
             
            
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
                	break; // replace by continue 
               }//timeout on loop reception go back to the loop's begining into a new epoch 
            
            //TODO Synchronization
            //@ assert round == New_Leader; 
            
            //Move away this it the " normal " end of a leadership when it loses quorum
                //@ old_epoch = epoch;
                epoch++;
                //@ old_round = round;
                round = CEpoch;
                
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
           
                list_dispose_mbox(mbox);
                mbox = NULL;
            
            
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
                	break; // replace by continue 
            	 
            	 }//timeout go back to the loops begining into a new epoch
            	 
            	
            	 
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
                
                //TODO Synchronization 	 	 
            	
        	//Move away this it the " normal " end of a folwers when it lost leader 
         	//@ old_epoch = epoch;
                epoch++;
                //@ old_round = round;
                round = CEpoch;
                //@ close tag_leq(old_epoch,old_round,epoch,round);
                //@assert tag_leq(old_epoch,old_round,epoch,round);
                //@ open tag_leq(old_epoch,old_round,epoch,round);
        	
           
        }//END FOLLOWER
        
      }//END LOOP LEADER_ELECTION 
      list_dispose(log);

        return 1;
    }

    

