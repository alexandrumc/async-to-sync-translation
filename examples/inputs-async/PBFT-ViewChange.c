#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include<stdbool.h>
#include<time.h> 


#define TIMEOUT	3

typedef struct Msg {
	int round;
	int view;
	// ViewChange msg
	int checkpoint;
	struct List * checkpoint_certificate_set;
	struct List * prepared_set;
	int pid;
	// NewView msg
	struct List * viewchange_certificate_set;  
	struct List * preprepared_set;
} msg;

typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

enum round_typ {ViewChange_ROUND, NewView_ROUND, AUX_ROUND} ;

time_t current_time;
 
bool timeout(){
    double diff_t;
    time_t now;
    time(&now);
    
    diff_t = difftime(now, current_time);
    
    if(diff_t >= TIMEOUT) {
        printf("TIMEOUT! %ld\n", (long)diff_t);
        return true;
    }else {
        return false;   
    }
}

void reset_timeout(){
    time(&current_time);
    printf("Reset time = %ld\n", (long)current_time);
}

bool newview_signed(){
    return true;
}

int primary(int n, int view){
    return view % n;
}

void out(int v1, int v2);

bool all_same(list *mbox, int view){
    return true;
}

void send(msg* message, int pid){
    printf("Sending message\n");
}


msg* recv()
{
    printf("Waiting for messages\n");
    
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) {
    abort();
    }
    //@ assume(m->ballot > INT_MIN && m->ballot < INT_MAX);
    return m;
}


void dispose(msg* c)
{
    free(c);
}

void list_dispose(list* l)
{
    list* current = l;  
    list* next;  
      
    while (current != NULL)  
    {  
        next = current->next;  
        dispose(current->message);
        free(current);  
        current = next;  
    }  
          
    l = NULL;  
}


int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
{
	reset_timeout();
    
    int n = argc;
    int to_all = n+1;
      
    enum round_typ round;
    int view;
    int pid,leader;
    list* mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    view = 0;

    while(true)
    {
        round = ViewChange_ROUND;
        mbox = NULL;
	   
	    // Everyone is watching for a timeout to occur
        while(true){	
            if (timeout())
            {
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                abort();
                }
                m->view = view;
                m->round = ViewChange_ROUND;

                send(m, to_all);
                dispose(m);
                m = NULL;
                break;
            }
        }
        
        // We detected a timeout, we wait for a certificate from the other replicas
        while(true)
        {   
            m = recv();
            // Received a valid message
            if (m != NULL && m->view>=view && m->round == ViewChange_ROUND){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message =m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else { mbox_new->size =1 ;}
                mbox_new->next = mbox;

                mbox = mbox_new;
                
                // If I receive 2f messages...
                if(mbox!=NULL && mbox->size >= 2*(n + 1) / 3 && all_same(mbox,m->view) ) {
                    break;
                }

            }else{ dispose(m); }

        }
        
        // A timeout was properly certified, the new primary advertise the NewView to others
        list_dispose(mbox);
        mbox=NULL;
        
        if(primary(n,m->view)==pid) {
            
            printf("I am %i, the primary of the new view\n", pid);
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->view = view;
            m->round = NewView_ROUND;

            send(m, to_all);
            dispose(m);
            m = NULL;
            
        }else{
         
            while(true)
            {
                m = recv();
                // Received a valid confirmation of the NewView
                if (m != NULL && m->view>=view && m->round == NewView_ROUND && newview_signed(m)){
                    break;
                }else{ dispose(m); }
    
            }   
        
        }

        view++;
        reset_timeout();

    }   
    return 1;
}
