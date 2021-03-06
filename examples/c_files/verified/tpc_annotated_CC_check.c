#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include "functions.h"

//typedef struct Msg {
//    int round; //round
//    int phase;  //phase
//    int payload;
//    int response;
//    int sender;
//    int commit ;
//} msg;

/*@
 predicate tag_leq(int p1, int r1, int p2, int r2) = (p1 < p2) || (p1==p2 && r1<=r2) ;
 @*/


//typedef struct List{
//    msg * message;
//    struct List * next;
//    int size;
//} list;


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
   
    phase = 0;
     //@ old_phase = phase-1;

    round = AUX_ROUND;
    //@ old_round = round;
    
     int response;
     int count = 1;
     int current_command;
     int commit = -1;

     int *log;
    
    round = FIRST_ROUND;

	while (1)
     
    {
        //@ assert (old_phase < phase && round = FIRST_ROUND)
        
        
        //@ old_round = round;
        //@ old_phase = phase;
        
        round = FIRST_ROUND;
        //@assert tag_leq(old_phase,old_round,phase,round);
      


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
		while (1) {
			m = recv();
            		if (m != NULL && m->phase == phase && m->round == FIRST_ROUND){
                
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
                		mbox = mbox_new;
                		}
                
            		}else free(m);

			if ((mbox->size >= 1 && mbox->message->sender == leader(phase,n)))
				break;
		}
        //@ assert mbox_tag_geq(mbox, phase, round);
        // phase = max(mbox); it would be allowed and the assert checked only after
        //@ assert max_tag(mbox, phase, round);
        
    
		if ((mbox->size >= 1 && mbox->message->sender == leader(phase,n) && mbox->message->payload >0 )) {
            response = rand_bool();
			current_command = mbox->message->payload;
		}

        //@ old_round = round;
		round = SECOND_ROUND;
        //@assert tag_leq(old_phase,old_round,phase,round);

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

        list_dispose(mbox);
        reset_timeout();
        mbox = NULL;
        
        if (pid == leader(phase,n)) {
			while (1) {
				m = recv();
				if (m != NULL && m->phase == phase && m->round == SECOND_ROUND){
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
                    mbox = mbox_new;
                    }
				}
				if (mbox->size == n)
					break;
			}
            
            //@ assert mbox_tag_geq(mbox, phase, round);
            // phase = max(mbox); it would be allowed and the assert checked only after
            //@ assert max_tag(mbox, phase, round);
            
            commit = all_agree(mbox);

		}

        //@ old_round = round;
		round = THIRD_ROUND;
        //@assert tag_leq(old_phase,old_round,phase,round);

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
        
		while (1) {
			m = recv();
            if (m != NULL && m->phase == phase && m->round == THIRD_ROUND){
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
                mbox = mbox_new;
                }
			}
            
			if ((mbox->size >= 1 && mbox->message->sender == leader(phase,n)))
				break;

		}
        
        //@ assert mbox_tag_geq(mbox, phase, round);
        // phase = max(mbox); it would be allowed and the assert checked only after
        //@ assert max_tag(mbox, phase, round);

		if (mbox->message->commit == 1) {
			out(current_command,pid);
		}

        //@ old_round = round;
		round = FOURTH_ROUND;
        //@assert tag_leq(old_phase,old_round,phase,round);
        
		msg *m = (msg *) malloc(sizeof(msg));
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
            
			while (1) {
				m = recv();
                if (m != NULL && m->phase == phase && m->round == FOURTH_ROUND){
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
                    mbox = mbox_new;
                    }
                }
				
				if (mbox->size == n)
					break;
			}
            
            //@ assert mbox_tag_geq(mbox, phase, round);
            // phase = max(mbox); it would be allowed and the assert checked only after
            //@ assert max_tag(mbox, phase, round);
           
            
            //@ old_phase = phase;
			phase = phase + 1;
		}
		else {
              //@ old_phase = phase;
			phase = phase + 1;
		}
          //@ old_round = round;
		round = FIRST_ROUND;
        //@assert tag_leq(old_phase,old_round,phase,round);
    
	}

}
