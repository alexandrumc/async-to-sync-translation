#include "stdlib.h"
#include<stdio.h>
#include<limits.h>
#include"Zab-Discovery_Synchronization_Broadcast2.h"
int main(int argc, char **argv)
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
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
   while (true)
      {
        if(pid == leader(epoch,n)){
            round = CEpoch;
            reset_timeout();
            mbox = NULL;
            while(true)
            {
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == CEpoch){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) abort();
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }
                else free(m);
                if (timeout()){
                  break;
                  }
                if(mbox != NULL && mbox->size > n/2){
                    break;
                    }
            }
            if(mbox != NULL && mbox->size > n/2){
                epoch = max_epoch(mbox);
                epoch++;
                round = NewEpoch;
            }
            else {
                list_dispose_mbox(mbox);
                mbox = NULL;
                epoch++;
                round = CEpoch;
                break;
            }
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->epoch = epoch;
            m->round = NewEpoch;
            send(m, to_all);
            free(m);
            m = NULL;
            round = Ack_E;
             list_dispose_mbox(mbox);
             mbox = NULL;
             while(true)
            {
                m = recv();
                if (m != NULL && m->epoch == epoch && m->round == Ack_E){
                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) abort();
                    mbox_new->message =m;
                    if(mbox!=0)
                        mbox_new->size = mbox->size + 1;
                    else mbox_new->size =1 ;
                    mbox_new->next = mbox;
                    mbox = mbox_new;
                }
                else free(m);
                if (timeout()){
                  break;
                  }
                if(mbox != NULL && mbox->size > n/2){
                    break;
                    }
            }
               if(mbox != NULL && mbox->size > n/2){
                   lastIndex = max_log_size(mbox);
                   struct arraylist* old_log = log;
                          log = longest_log(mbox, lastIndex);
                   list_dispose(old_log);
                   round = New_Leader;
               }
               else {
                 list_dispose_mbox(mbox);
                 mbox = NULL;
                 epoch++;
                 round = CEpoch;
                 break;
               }
                epoch++;
                round = CEpoch;
                list_dispose_mbox(mbox);
                mbox = NULL;
        }
        else{
         round = CEpoch;
         m = (msg *) malloc(sizeof(msg));
             if(m==0) abort();
             m->epoch = epoch;
             m->round = CEpoch;
             send(m, leader(epoch,n));
             free(m);
             m = NULL;
             round = NewEpoch;
             mbox= NULL;
             reset_timeout();
             while(true)
             {
                m = recv();
                if (m != NULL && m->epoch >= epoch && m->round == NewEpoch){
                  mbox_new = (list*) malloc(sizeof(list));
                  if(mbox_new==0) abort();
                  mbox_new->message =m;
                  if(mbox!=0)
                       mbox_new->size = mbox->size + 1;
                  else mbox_new->size =1 ;
                  mbox_new->next = mbox;
                  mbox = mbox_new;
              } else free(m);
              if (timeout()) break;
                 if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                  break;
              }
             }
              if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
               epoch = mbox->message->epoch;
                 round = Ack_E;
              }
              else {
                 list_dispose_mbox(mbox);
                 mbox = NULL;
                 epoch++;
                 round = CEpoch;
                 break;
              }
             list_dispose_mbox(mbox);
                mbox = NULL;
             m = (msg *) malloc(sizeof(msg));
             if(m==0) abort();
             m->epoch = epoch;
             m->round = Ack_E;
             m->history = log;
             m->history_lenght = lastIndex;
             send(m, leader(epoch,n));
             free(m);
             m = NULL;
                epoch++;
                round = CEpoch;
        }
      }
      list_dispose(log);
        return 1;
    }
