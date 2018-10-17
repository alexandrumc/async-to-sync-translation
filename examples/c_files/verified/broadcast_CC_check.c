#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include "broadcast-functions.h"
#include "arraylist.h"






int broadcast(int argc, int pid, struct arraylist * log, int lastIndex,  int leader, int epoch)
//@ requires  arraylist(log, ?log_data) &*& 0<= lastIndex &*& lastIndex == length(log_data)-1;
//@ ensures true;
{

    
    
    int n = argc;
    int to_all = n+1;
    
    
    enum round_typ_B round; //round
    int i; //phase
    int lab = BCAST; //upper_round
    // epoch upper_phase;
    
    //@ int old_phase;
    //@ int old_phase_upper = epoch;
    //@ round_typ_A old_round_upper = BCAST;
    //@ round_typ_B old_round;
    
    
    struct List* mbox;
    struct List *mbox_new;
    msg *m =NULL;

    round = FIRST_ROUND;
 

    
	if (pid == leader) {
		ltype * lastEntry = list_get(log,lastIndex); 
        if (lastEntry!= NULL && lastEntry->commit == true) {
            lastIndex++;
            ltype * newEntry = create_ltype(in(),false);
            list_add(log,newEntry);
        }
	}

    

    
       i = lastIndex;
    
	while (true) 
	{
        //@ assert (old_round_upper == lab &*& old_phase_upper == epoch &*& old_phase + 1 = i &*& round == FIRST_ROUND);
	
	round = FIRST_ROUND;
        //@ old_round = round;
        //@ old_phase = i;
        
        //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
        
		if (pid == leader) {

			m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
			m->i = lastIndex;
			m->round = round;
			m->epoch = epoch;
            m->lab = BCAST;
            m->sender = leader;
            
            ltype * entry = list_get(log,lastIndex);
            if(entry != NULL) m->op = entry->op;
            
            //free(entry);
            
            
             //@ assert (m->lab == lab &*& m->epoch == epoch &*& m->i == i &*& m->round == round);
			send(m, to_all);
            dispose(m);
            m = NULL;
		}

        mbox = NULL;
        
        while (true) 
        
        {
            m = recv();
            if ( m!=NULL && m->i == lastIndex && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
            }else free(m);
            
            if (mbox != NULL && mbox->size >= 1 && mbox->message!=NULL && mbox->message->sender == leader)
                break;
            
            if (timeout())
                break;
        }

        //@ assert eq_val_list_pred(phase_upper, round_upper,phase,round,mbox);
		
        
        if (mbox != NULL && mbox->size >= 1 && mbox->message!=NULL && mbox->message->sender == leader){
            
            //@ assert eq_val_list_pred(phase_upper, round_upper,phase,round,mbox);
            if(pid != leader){
                ltype * newEntry = create_ltype(mbox->message->op,false);
                list_add(log,newEntry);
            }
        
            //@ old_round = round;
            round = SECOND_ROUND;
            
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->i = i;
            m->round = round;
            m->epoch = epoch;
            m->lab = BCAST;
            m->sender = pid;
		
            //@ assert (m->lab == lab &*& m->epoch == epoch &*& m->i == i &*& m->round == round);

            send(m, leader);
            dispose(m);
            m=NULL;

            if (pid == leader) {
				while (true) {
					m = recv();

					if (m!=NULL && m->i == lastIndex && m->epoch == epoch && m->round == round && m->lab == BCAST) {
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) abort();
                        mbox_new->message =m;
                        if(mbox!=0)
                            mbox_new->size = mbox->size + 1;
                        else  mbox_new->size =1 ;
                        mbox_new->next = mbox;
                        mbox = mbox_new;
                    }else free(m);

					if (timeout())
					    break;

					if (mbox != NULL && mbox->size > n/2)
					    break;
				}

                //@ assert eq_val_list_pred(phase_upper, round_upper,phase,round,mbox);
				if (mbox != NULL && mbox->size > n/2) {
                    //@ assert eq_val_list_pred(phase_upper, round_upper,phase,round,mbox);

                    ltype *logi = list_get(log,i);
                    list_remove_nth(log, i);
                    if(logi != 0)
                        logi->commit = true;
                    list_add(log,logi);
					out(logi);
				}
				else {
				    break;
				}
                
                //@ old_round = round;
				round = THIRD_ROUND;
                //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
			}
			else {
                //@ old_round = round;
				round = THIRD_ROUND;
                //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
			}

			if (pid == leader) {
				m = (msg *) malloc(sizeof(msg));
                if(m==0) abort();
				m->i = i;
				m->round = round;
				m->epoch = epoch;
				m->lab = BCAST;
				m->sender = pid;
                
                //@ assert (m->lab == lab &*& m->epoch == epoch &*& m->i == i &*& m->round == round);

				send(m, to_all);
                dispose(m);
                m=NULL;
			}

			while (true) {
				m = recv();
				if (m!=NULL && m->i == lastIndex && m->epoch == epoch && m->round == round && m->lab == BCAST) {

                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) abort();
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else  mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }else free(m);

                if (mbox != NULL && mbox->size >= 1 && mbox->message!=NULL && mbox->message->sender == leader)
                    break;
                
                if (timeout())
                    break;

			}
            //@ assert eq_val_list_pred(phase_upper, round_upper,phase,round,mbox);

			if (mbox != NULL && mbox->size >= 1) {
                
                //@ assert eq_val_list_pred(phase_upper, round_upper,phase,round,mbox);

                
                ltype *logi = list_get(log,i);
                list_remove_nth(log, i);
                if(logi != 0)
                    logi->commit = true;
                list_add(log,logi);
                out(logi);
                
                if(pid == leader){
                    lastIndex++;
                    ltype * newEntry = create_ltype(in(),false);
                    list_add(log,newEntry);
                }
                
                //@ old_phase = i;
                i++;
                
                //@ old_round = round;
				round = AUX_ROUND;
                
                //@ assert tag_strict_leq(old_phase_upper, old_round_upper,old_phase,old_round, epoch,lab, i,round);
                
            }else break;
            
        }else{break;}
	}
}
