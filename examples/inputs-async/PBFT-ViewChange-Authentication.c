#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include<stdbool.h>

typedef struct Msg {
	int round;
	int view;
	// ViewChange data
	int checkpoint;
	struct List * checkpoint_certificate_set;
	struct List * prepared_set;
	int pid;
	// ViewChangeAck data
	int pid_ack;
	// NewView data
	struct List * viewchange_certificate_set;  
	struct List * preprepared_set;
} msg;

typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

enum round_typ {ViewChange_ROUND, ViewChangeAck_ROUND, NewView_ROUND, AUX_ROUND} ;

int primary(int n, int view){
    return view % n;
}

void out(int v1){

}

int timeout(){
    return 1;
}
//@ requires emp;
//@ ensures emp;

void send(msg* message, int pid);
//@ requires true;
//@ ensures true;


msg* recv()
{
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) {
    abort();
    }

    return m;
}


void dispose(msg* c)
{
    free(c);
}

int all_same(list* mbox, int view){

    return 1;
}

void list_dispose(list* l);

int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
{
    int pid;
    int to_all = n+1;
      
    enum round_typ round;
    int view;

    list* mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    view = 0;
    
    while(true)
    {
        round = ViewChange_ROUND;
        mbox = NULL;
         
	    // A timeout has been detected
        m = (msg *) malloc(sizeof(msg));
        if(m==0) {
        abort();
        }
        m->view = view;
        m->round = ViewChange_ROUND;

        send(m, to_all);
        dispose(m);
        m = NULL;
        mbox = NULL;
        
        while(true)
        {   
            m = recv();
            
            // Received a valid message
            if (m != NULL && m->view>=view && m->round == ViewChange_ROUND){
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) {
                abort();
                }
                mbox_new->message = m;
                if(mbox!=0){mbox_new->size = mbox->size + 1;}
                else { mbox_new->size =1 ;}
                mbox_new->next = mbox;

                mbox = mbox_new;

            }else{ dispose(m); }
            
            if (timeout()){ break; }
            
            if(mbox!=NULL && mbox->size > 2*n/3 && all_same(mbox,view) ) {
                break;
            }

        }
        
        if(mbox!=NULL && mbox->size > 2*n/3 && all_same(mbox,view) ) {  
            view = mbox->message->view;  
            round = ViewChangeAck_ROUND;
            
            while(true)
            { 
                mviewchange = mbox->message;
                // Send the ViewChangeAck to the new primary
                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                abort();
                }
                m->view = view;
                m->pid = pid;
                m->pid_ack = mviewchange->pid;
                m->round = ViewChangeAck_ROUND;
        
                send(m, primary(n,view));
                dispose(m);
                
                mviewchange = mbox->next;
                
                if(mviewchange == NULL) {
                    break;
                }
            }
                
            list_dispose(mbox);
        }
        
        round = ViewChange_ROUND;
        
        if(primary(n,view)==pid){
            
            m = (msg *) malloc(sizeof(msg));
            if(m==0) {
            abort();
            }
            m->view = view;
            m->round = NewView_ROUND;
            
            send(m, to_all);
            dispose(m);
            m = NULL;
            
            out(view);
        
        }else{

            while(true)
            {
    
                m = recv();
                // Received a valid message
                if (m != NULL && m->view>=view){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {
                    abort();
                    }
                    mbox_new->message = m;
                    if(mbox!=0){mbox_new->size = mbox->size + 1;}
                    else { mbox_new->size =1 ;}
                    mbox_new->next = mbox;
    
                    mbox = mbox_new;
    
                }else{ dispose(m); }
                
                // Received a valid confirmation of the NewView
                if (m != NULL && mbox->size ==1 && mbox->next == NULL && mbox->message->round == NewView_ROUND && view == mbox->message->view && certified_newview(m)){
                    break;
                }
    
                if (timeout()){ break; }
    
            }   

            if (m != NULL && mbox->size ==1 && mbox->next == NULL && mbox->message->round == NewView_ROUND && view == mbox->message->view && certified_newview(m)){
                out(view);   
            }
        
        }
        
        view++;
        round = ViewChange_ROUND;
    
    
        

    }   
    return 1;
}