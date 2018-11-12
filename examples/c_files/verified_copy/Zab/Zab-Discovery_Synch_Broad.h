#ifndef Leader_election_Zab_h
#define Leader_election_Zab_h 
#include <stdio.h>
#include "stdlib.h"
#include <stdbool.h>
#include "broadcast-function-nested.h"
struct arraylist;
struct arraylist *create_arraylist() ;
struct arraylist *create_arraylist2() ;
ltype *list_get(struct arraylist *a, int i);
int list_length(struct arraylist *a);
void list_add(struct arraylist *a, void *v);
void list_remove_nth(struct arraylist *a, int n);
void list_dispose(struct arraylist* a);
enum round_typ_A {CEpoch, NewEpoch, Ack_E, New_Leader,Ack_LD, BCAST} ;
typedef struct Msg {
    int round;
    int pid;
    int epoch;
    struct arraylist *history;
    int history_lenght;
    int sender;
    int op;
    int i;
    int lab;
} msg;
typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;
int max_epoch(struct List* mbox);
void list_dispose_mbox(struct List *l);
void dispose_msg(struct Msg *msg);
bool reset_timeout();
int leader(int phase, int net_size);
bool timeout();
msg* recv();
void send(msg* message, int pid);
int max_log_size(struct List* mbox);
struct arraylist* longest_log(struct List* mbox, int lastIndex);
void list_dispose_data(struct arraylist* a);
#endif
