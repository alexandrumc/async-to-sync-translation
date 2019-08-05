#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include<stdbool.h>

typedef struct Msg {
	int round;
	int view;
} msg;

typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

enum round_typ {StartViewChange_ROUND, DoViewChange_ROUND, StartView_ROUND, AUX_ROUND} ;

int primary(int n, int view);

void out(int v1);

int timeout();
//@ requires emp;
//@ ensures emp;

void send(msg* message, int pid);
//@ requires true;
//@ ensures true;

msg* recv();

void dispose(msg* c);

int all_same(list* mbox, int view);

void list_dispose(list* l);

int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
{
    /* Init */
    int pid;
    int to_all = n+1;

    enum round_typ round;
    int view;

    list* mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    view = 0;

    /* First round */
    round = StartViewChange_ROUND;
    m = (msg *) malloc(sizeof(msg));
    m->view = view;
    m->round = StartViewChange_ROUND;
    send((void*)m, to_all); 
	
    while(true)
    {

        while(true)
        {
            m = recv();

            // Received a valid message
            if (m != NULL){
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

            if(round == StartViewChange_ROUND && follower(n, view) == pid && collected_all_start_view_change(mbox,view) ) {
                break;
            }

            if(round == StartView_ROUND && follower(n, view) == pid && m->round == StartView_ROUND ) {
                break;
            }

            if(round == StartViewChange_ROUND && primary(n, view) == pid && m->round == DoViewChange_ROUND) {
                break;
            }

            if(round == DoViewChange_ROUND && primary(n, view) == pid && collected_all_do_view_change(mbox,view) ) {
                break;
            }


        }
        
        /* 
            I am a follower and I received all the f StartViewChange messages, 
            I advertise this to the new primary
            I go to StartView round and wait for the new primary to confirm the new view 
        */
        if(round == StartViewChange_ROUND && follower(n, view) == pid && collected_all_start_view_change(mbox,view) ) {
            round = DoViewChange_ROUND;

            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = DoViewChange_ROUND;

            send((void*)m, primary(n, view));

            round = StartView_ROUND;

	        continue;
        }

        /*
            I am a follower, the new primary confirmed the new view
        */

        if(round == StartView_ROUND && follower(n, view) == pid && m->round == StartView_ROUND) {

            out(view);

            // I prepare for the next view change 
            round = StartViewChange_ROUND;
            view++;
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = StartViewChange_ROUND;
            send((void*)m, to_all); 
            
            continue;
        }

        /*
            I am the new primary and just started the protocol, I wait for f+1 DoViewChange messages
        */
        if(round == StartViewChange_ROUND && primary(n, view) == pid && m->round == DoViewChange_ROUND) {
            round = DoViewChange_ROUND;

	        continue;
        }

        /*
            I am the new primary and I collected f+1 DoViewChange messages, I advertise the followers
        */
        if(round == DoViewChange_ROUND && primary(n, view) == pid && collected_all_do_view_change(mbox,view) ) {
            round = StartView_ROUND;

            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = StartView_ROUND;

            send((void*)m, to_all);
            out(view);
    
            // I prepare for the next view change 
            round = StartViewChange_ROUND;
            view++;
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = StartViewChange_ROUND;
            send((void*)m, to_all); 

            continue;
        }

        
    }
    return 1;
}
