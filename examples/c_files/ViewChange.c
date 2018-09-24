//
//  ViewChange.c
//  test
//
//  Created by C on 18/09/2018.
//  Copyright © 2018 C. All rights reserved.
//

#include "ViewChange.h"
typedef struct _msg {
    int round;
    int pid;
    int epoch;
    int* history;
    int history_lenght;
    int commit_nb;
    int sender;
} msg;

typedef enum ROUNDS {START_VIEW_CHANGE, DO_VIEW_CHANGE, START_VIEW, PREPARE_OK};

int ViewChange(int pid, int num_processes){
    int round = START_VIEW_CHANGE;
    int view = 0;
    int commit;
    int *history;
    int history_lenght;
    
    int leader;
    
    msg* mbox = malloc(2*num_processes, sizeof(msg));
    int num_mbox= 0;
    int retry;
    
    while (1) {
        
        round = START_VIEW_CHANGE;
        view++;
        leader = view % num_processes;
        
        if(rand()%2 == 0) {
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = START_VIEW_CHANGE;
            send(m, all);
        }
        
        
        // Empty mbox
        memset(mbox,0,sizeof(mbox));
        num_mbox = 0;
            
        // receive
        while(true){
            m = recv(); //typed receives otherwise more complex code to do the casts
            if (m != NULL && m.view == view && m.round == START_VIEW_CHANGE){
                    mbox[num_mbox] = m;
                    num_mbox++;
            }
            if (m != NULL && m.view > view) break;
            if (timeout() || (num_mbox >= num_processes/2))
                    break;
        }
        
            
        if (num_mbox >= num_processes/2) {
                round = DO_VIEW_CHANGE;
        }else{
            if(m.view > view){
                        view = m.view; //jump
                        round = START_VIEW_CHANGE;
                        continue;
            }else{
                //round = START_VIEW_CHANGE;
                // increase view or not ???
                //continue;
                //AUX_ROUND ?
                }
        }
        
        if(round == DO_VIEW_CHANGE){
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = DO_VIEW_CHANGE;
            m->history = history;
            m->history_lenght = history_lenght;
            m->commit_nb = commit_nb;
            m->old_view = old_view;
            send(m, leader);
        }
        
        if(leader == pid){
        
            // Empty mbox
            memset(mbox,0,sizeof(mbox));
            num_mbox = 0;
        
            while (true){
                m = recv();
                if (m != NULL && m.view >= view && m.round == round) {
                    mbox[num_mbox] = m;
                    num_mbox++;
                }
                if (timeout() || num_mbox >= num_processes/2){
                break;
                }
            }
        
            if(num_mbox >= num_processes/2) {
                view = get_view(mbox); // jump
                history = get_longest_history(mbox);
                commit_nb = get_biggest_commit(mbox);
                round = START_VIEW;
            }else {
            round = START_VIEW_CHANGE;
            continue;
            }
        }
        
        
        if(leader == pid){
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = round;
            m->history = history;
            m->history_lenght = history_lenght;
            m->commit_nb = commit_nb
            send (m, to_all);
        }
        
        // Empty mbox
        memset(mbox,0,sizeof(mbox));
        num_mbox = 0;
        
        while (true){
            m = recv();
            if (m != NULL && m.view >= view && m.round == round) {
                mbox[num_mbox] = m;
                num_mbox++;            }
            if (timeout() || num_mbox >= 1){
                break;
            }
        }
 
        if(num_mbox >=1) {
            view = mbox[num_mbox].view; // jump
            round = mbox[num_mbox].round; // into view and round start-view
            history = mbox[num_mbox].history;
            history_lenght = mbox[num_mbox].history_lenght;
            if(unprepared_log(history)) round = PREPARE_OK;
        }else {
            round = START_VIEW_CHANGE;
            continue;
        }
        
        
        if(round == PREPARE_OK){
            m = (msg *) malloc(sizeof(msg));
            m->view = view;
            m->round = round;
            m->history = history;
            m->history_lenght = history_lenght;
            m->commit_nb = commit_nb;
            send (m, leader);
        }
        if (round == START_VIEW || round == PREPARE_OK) NormalOp();
    }
}
