#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include "functions.h"



/*@
 predicate tag_leq(int p1, int r1, int p2, int r2) = (p1 < p2) || (p1==p2 && r1<=r2) ;
 
 predicate tag_strict_leq(int p1, int r1, int p2, int r2) = (p1+1 == p2) || (p1==p2 && r1<=r2) ;
 @*/



typedef struct Msg {
    int round;
    int phase;
    int payload;
    int response;
    int sender;
    int commit;
} msg;


typedef struct List {
    msg *message;
    struct List *next;
    int size;
} list;


void out(int v1, int v2);


int in();


int rand_bool();


msg *recv();

void dispose(msg *c);


void list_dispose(struct List *l);


void list_dispose_no_data(struct List *l);

void dispose_list(list *c);

void send(msg *message, int pid);


int leader(int phase, int net_size);


int all_agree(struct List *l);



enum round_typ {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, FOURTH_ROUND, AUX_ROUND} ;


int main(int argc, char **argv) //@ : main
//@ requires true;
//@ ensures true;
{
    
    int n = argc;
    int to_all = n+1;
    
    enum round_typ round;
    int phase;
    //@ int old_round; 
    //@ int old_phase;
    
    int pid;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    //@ old_phase = -1;
    phase = 0;
    //@ old_round = AUX_ROUND;
    round = AUX_ROUND;
    
     int response;
     int count = 1;
     int current_command;
     int commit = -1;

     int *log;
     
     int leader;
    
    round = FIRST_ROUND;

	while (1)
    //@invariant (old_phase +1 == phase && round == FIRST_ROUND);
    {
        //tag = (phase, round);
        
        //@ old_round = round;
        //@ old_phase = phase;
        
	round = FIRST_ROUND;
        
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@ assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);


	if (pid == leader(phase,n)) {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->phase = phase;
			m->round = round;
			m->payload = in();
			m->sender = pid;
            m->response = -1;
            m->commit = -1;
            
            //@assert(m->phase == phase && m->round == round);
            send(m, to_all);
            dispose(m);
            m = NULL;
		}

    reset_timeout();
    mbox = NULL;
    //@ close lseg(mbox,0,nil);
    //@ close foreach(nil,eq_list(phase,round));
    //@ close mbox_tag_EQ(mbox, phase, round);
        
        
	while (1)
        //@ invariant mbox_tag_EQ(mbox, phase, round);
        {
			
			//@ open mbox_tag_EQ(mbox, phase, round);
			
			m = recv();
            		if (m != NULL && m->phase == phase && m->round == FIRST_ROUND){
                
                		//@ open lseg(mbox,0,?v);
                		//@ close eq_list(phase,round)(m);
                		
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
                		//@ close lseg(mbox,0, v);
                		mbox = mbox_new;
                        //@ close lseg(mbox,0, cons(m,v));
                		//@ close foreach(cons(m,v), eq_list(phase,round));
                		}
                
            		}else free(m);

		        
		        leader =  leader(phase,n);
		        
		        //@open lseg(mbox, 0, ?nv);
		        //@open foreach(nv,eq_list(phase,round)); 
		       
		        
			if (mbox != NULL && mbox->size >= 1) {
			 	//@open eq_list(phase,round)(head(nv));
				if (mbox->message != NULL && mbox->message->sender == leader){
					//@close eq_list(phase,round)(head(nv));
					//@close foreach(nv,eq_list(phase,round)); 
					//@close lseg(mbox, 0, nv);
					//@close mbox_tag_EQ(mbox, phase, round);
					break;
				}
				//@close eq_list(phase,round)(head(nv));
			}

			//@close foreach(nv,eq_list(phase,round)); 
			//@close lseg(mbox, 0, nv);
			//@close mbox_tag_EQ(mbox, phase, round);
			
		}
		
		
        //@ assert mbox_tag_EQ(mbox, phase, round);

        
        //@open mbox_tag_EQ(mbox, phase, round);
        //@open lseg(mbox, 0, ?nv);
        //@open foreach(nv,eq_list(phase,round));
        //@open eq_list(phase,round)(head(nv));
        leader = leader(phase,n);
		if ((mbox->size >= 1 && mbox->message != NULL && mbox->message->sender == leader && mbox->message->payload >0 )) {
            		response = rand_bool();
			current_command = mbox->message->payload;
		}
		
        //@close eq_list(phase,round)(head(nv));
        //@close foreach(nv,eq_list(phase,round));
        //@close lseg(mbox, 0, nv);
        //@ lemma_EQ_list_to_alloc_list(mbox, phase, round);

        //@ old_round = round;
        //@ old_phase = phase;
        round = SECOND_ROUND;
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);
       

        m = (msg *) malloc(sizeof(msg));
        if (m==0) {
        abort();
        }
        m->phase = phase;
        m->round = round;
        m->payload = current_command;
        m->sender = pid;
        m->response = response;
        m->commit = -1;
    
        //@assert(m->phase == phase && m->round == round);
        send(m, leader(phase,n));
        dispose(m);
        m = NULL;

       
        
        if (pid == leader(phase,n)) {
            
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            //@ close lseg(mbox,0,nil);
            //@ close foreach(nil,eq_list(phase,round));
            //@ close mbox_tag_EQ(mbox, phase, round);
            while (1)
                //@ invariant mbox_tag_EQ(mbox, phase, round);
            {
                m = recv();
            		//@ open mbox_tag_EQ(mbox, phase, round);
                if (m != NULL && m->phase == phase && m->round == SECOND_ROUND){
                	//@ open lseg(mbox,0,?v);
                	//@ close eq_list(phase,round)(m);
                
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
                            //@ close lseg(mbox,0, v);
                            mbox = mbox_new;
                            //@ close lseg(mbox,0, cons(m,v));
                            //@ close foreach(cons(m,v), eq_list(phase,round));
                            }
                }else free(m);
            
            	//@open lseg(mbox, 0, ?nnv);
	
            	if (mbox != NULL && mbox->size == n){
                   	//@close lseg(mbox, 0, nnv);
		    	break;
            		}
            	//@close lseg(mbox, 0, nnv);
                //@ close  mbox_tag_EQ(mbox, phase, round);
            }
	
            //@ close  mbox_tag_EQ(mbox, phase, round);
            
            //@ assert mbox_tag_EQ(mbox, phase, round);
           
            //@ open mbox_tag_EQ(mbox, phase, round);
            //@ lemma_EQ_list_to_alloc_list(mbox, phase, round);
            commit = all_agree(mbox);
        }
		

	


        //@ old_round = round;
        //@ old_phase = phase;
        round = THIRD_ROUND;
	
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@ assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);

	if (pid == leader(phase,n)) {
            m = (msg *) malloc(sizeof(msg));
            if (m==0) {
            abort();
            }
            m->phase = phase;
            m->round = round;
            m->payload = current_command;
            m->sender = pid;
            m->response = response;
            m->commit = commit;
            
            //@assert(m->phase == phase && m->round == round);
	    send(m, to_all);
            dispose(m);
            m = NULL;

	}
        
        
       
        list_dispose(mbox);
        reset_timeout();
        mbox = NULL;
        //@ close lseg(mbox,0,nil);
        //@ close foreach(nil,eq_list(phase,round));
        //@ close mbox_tag_EQ(mbox, phase, round);
        
		while (1)  
		 //@ invariant mbox_tag_EQ(mbox, phase, round);
		 {
			m = recv();
			 //@ open mbox_tag_EQ(mbox, phase, round);
            		if (m != NULL && m->phase == phase && m->round == THIRD_ROUND){
            			//@ open lseg(mbox,0,?v);
                		//@ close eq_list(phase,round)(m);
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
                		//@ close lseg(mbox,0, v);
                            	mbox = mbox_new;
                            	//@ close lseg(mbox,0, cons(m,v));
                            	//@ close foreach(cons(m,v), eq_list(phase,round));
                            	}
			}else free(m);
			
            		leader = leader(phase,n);
            		
            		//@ open lseg(mbox,0,?v);
            		//@ open foreach(v,eq_list(phase,round)); 
            		
		        
			if (mbox != NULL && mbox->size >= 1) {
				//@ open eq_list(phase,round)(head(v));
				if (mbox->message != NULL && mbox->message->sender == leader){
				//@close eq_list(phase,round)(head(v));
				//@close foreach(v,eq_list(phase,round)); 
				//@close lseg(mbox, 0, v);
				//@close mbox_tag_EQ(mbox, phase, round);
				break;
				}
				//@close eq_list(phase,round)(head(v));
			}
			
			//@close foreach(v,eq_list(phase,round)); 
			//@close lseg(mbox, 0, v);
			//@close mbox_tag_EQ(mbox, phase, round);

		}
        
         //@ assert mbox_tag_EQ(mbox, phase, round);
         
         //@open mbox_tag_EQ(mbox, phase, round);
         //@open lseg(mbox, 0, ?nv1);
         //@open foreach(nv1,eq_list(phase,round));
         //@open eq_list(phase,round)(head(nv1));

		if (mbox->message != NULL && mbox->message->commit == 1) {
			out(current_command,pid);
		}
		
	//@close eq_list(phase,round)(head(nv1));
        //@close foreach(nv1,eq_list(phase,round));
        //@close lseg(mbox, 0, nv1);
        //@ lemma_EQ_list_to_alloc_list(mbox, phase, round);

        //@ old_round = round;
        //@ old_phase = phase;
	round = FOURTH_ROUND;
		
	//@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);
        
	m = (msg *) malloc(sizeof(msg));
        if(m==0) {
        abort();
        }
        m->phase = phase;
        m->round = round;
        m->payload = current_command;
        m->sender = pid;
        m->response = response;
        m->commit = commit;

        //@assert(m->phase == phase && m->round == round);
	send(m, leader(phase,n));
        dispose(m);
        m = NULL;


	
	if (pid == leader(phase,n)) {
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            //@ close lseg(mbox,0,nil);
            //@ close foreach(nil,eq_list(phase,round));
            //@ close mbox_tag_EQ(mbox, phase, round);
            
            
	    while (1)  //@ invariant mbox_tag_EQ(mbox, phase, round);
		{
		m = recv();
		//@ open mbox_tag_EQ(mbox, phase, round);
                if (m != NULL && m->phase == phase && m->round == FOURTH_ROUND){
                //@ open lseg(mbox,0,?v);
                //@ close eq_list(phase,round)(m);
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {
                    abort();
                    }
                    mbox_new->message = m;
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
                    //@ close foreach(cons(m,v), eq_list(phase,round));
                    }
                }else free(m);
		//@open lseg(mbox, 0, ?nnv);
		if (mbox!= NULL && mbox->size == n){
			//@close lseg(mbox, 0, nnv);
			break;
			}
		//@close lseg(mbox, 0, nnv);
		//@ close  mbox_tag_EQ(mbox, phase, round);	
		}
		
            
           //@ close  mbox_tag_EQ(mbox, phase, round);
            
           //@ assert mbox_tag_EQ(mbox, phase, round);
           //@ open mbox_tag_EQ(mbox, phase, round);
           //@ lemma_EQ_list_to_alloc_list(mbox, phase, round);      
           
           list_dispose(mbox);
           mbox = NULL;
          //@ close lseg(mbox,0,nil);
          //@ close foreach(nil,eq_list(phase,round));
          //@ open lseg(mbox,0,nil);
          //@ open foreach(nil,eq_list(phase,round));
          
          
           //@ old_phase = phase;
			phase = phase + 1;
	}
	else {
		list_dispose(mbox);
                reset_timeout();
                mbox = NULL;
               //@ close lseg(mbox,0,nil);
               //@ close foreach(nil,eq_list(phase,round));
              
                //@ open lseg(mbox,0,nil);
          	//@ open foreach(nil,eq_list(phase,round));
          
            
              //@ old_phase = phase;
			phase = phase + 1;
		}
          //@ old_round = round;
		round = FIRST_ROUND;
		
        //@ close tag_strict_leq(old_phase,old_round,phase,round);
        //@ assert tag_strict_leq(old_phase,old_round,phase,round);
        //@ open tag_strict_leq(old_phase,old_round,phase,round);
    
	}

}
