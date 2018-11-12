#ifndef functions_h
#define functions_h 
#include <stdio.h>
#include "stdlib.h"
#include <stdbool.h>
enum round_typ_B {FIRST_ROUND, SECOND_ROUND, THIRD_ROUND, AUX_ROUND} ;
typedef struct MsgB {
    int op;
    int epoch;
    int lab;
    int i;
    int round;
    int sender;
} msgb;
typedef struct Ltype {
    int op;
    bool commit;
} ltype;
typedef struct ListB{
    msgb * message;
    struct ListB * next;
    int size;
} listb;
void out(ltype *v);
int in();
msgb* recv_msgb();
void dispose(msgb* c);
ltype * create_ltype(int op, bool b);
void listB_dispose_double(struct ListB *l);
void listB_dispose_no_data(struct ListB *l);
void send_msgb(msgb* message, int pid);
int all_agree(struct ListB* l);
#endif
