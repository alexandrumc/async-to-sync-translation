//
//  Leader-election-Zab.c
//
//
//  Created by Cezara on 22/10/2018.
//
#include "stdlib.h"
#include<stdio.h>
#include<limits.h>


enum round_typ {NewBallot_ROUND, AckBallot_ROUND, AUX_ROUND} ;



typedef struct Msg {
	int round;
	int ballot;
	int leader;
} msg;

typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;


/*@
 predicate msg_pred(struct Msg * msg) =
 msg == 0? true : msg->round |-> _ &*& msg->ballot |-> _ &*& malloc_block_Msg(msg);
 @*/

struct arraylist;

/*@
 predicate arraylist(struct arraylist *a; list<void*> vs);
 @*/

struct arraylist *create_arraylist() ;
//@ requires true;
//@ ensures true;


struct arraylist *create_arraylist2() ;
//@ requires true;
//@ ensures true;


int list_length(struct arraylist *a);
//@ requires true;
//@ ensures true;

void list_add(struct arraylist *a, void *v);
//@ requires true;
//@ ensures true;

void list_remove_nth(struct arraylist *a, int n);
//@ requires true;
//@ ensures true;



int all_same(list *mbox, int leader);
//@ requires true;
//@ ensures true;





/*@
 predicate tag_leq(int p1, int r1, int p2, int r2) = (p1 < p2) || (p1==p2 && r1<=r2) ;

 predicate tag_strict_leq(int p1, int r1, int p2, int r2) = (p1+1 == p2) || (p1==p2 && r1<=r2) ;
 @*/


/*@
 // Predicate describing a linked list of size one whose only value is vbigger or equal to max
 predicate mbox_geq_1(int max, struct List* l, int len) =
 (l == 0 && len == 0) ?
 true
 :
 len == 1 &*& l->message|-> ?msg &*& l->next |-> NULL &*& l->size |-> 1 &*& malloc_block_Msg(msg) &*& msg->round |-> ?v1 &*& msg->ballot |-> ?v2 &*& msg->leader |-> ?s2 &*& malloc_block_List(l) &*& max <= v2 ?
 true : false ;
 @*/

void list_dispose1(struct List *l);
//@ requires  mbox_geq_1(?v,l,?s);
//@ ensures emp;

void out(int v1, int v2);
//@ requires true;
//@ ensures true;

int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
{

    int n = argc;
    int to_all = n+1;


    struct arraylist *log;
    log = create_arraylist();
    int lastIndex = list_length(log);


    enum round_typ round;
    int ballot;

    int pid,leader;

    ballot = 0;
    round = NewBallot_ROUND;

    //@ int old_ballot = ballot-1;
    //@ enum round_typ_A old_round = round;


    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;

    round = NewBallot_ROUND;

    while (true)
        //@ invariant ((old_ballot<ballot && round == NewBallot_ROUND)) ;
    {

        if(pid == coord(n)){

            //@ old_ballot = ballot;
            round = NewBallot_ROUND;

            //@ old_round = round;

            //@ close  tag_leq(old_ballot,old_round,ballot,round);
            //@assert tag_leq(old_ballot,old_round,ballot,round);
            //@open  tag_leq(old_ballot,old_round,ballot,round);

            //@ assert round == NewBallot_ROUND;

            m = (msg *) malloc(sizeof(msg));
            if(m==0) {abort();}
            m->ballot = ballot;
            m->round = NewBallot_ROUND;
            m->leader = pid;

            //@assert(m->ballot == ballot && m->round == round);
            send(m, to_all);

            free(m);
            m = NULL;

            mbox= NULL;
            reset_timeout();

            //@ int len = 0;
            while(true)
                //@ invariant mbox==NULL && len ==0;
            {

                m = recv();
                if (m != NULL && m->ballot >= ballot && m->round == NewBallot_ROUND){

                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {abort();}
                    mbox_new->message =m;
                    if(mbox!=0){mbox_new->size = mbox->size + 1;}
                    else{mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    //@ len =1;
                    mbox = mbox_new;
                    //@ close mbox_geq_1(ballot,mbox,1);
                } else { //@ close mbox_geq_1(ballot,NULL,0);
                    free(m);}

                //@ open mbox_geq_1(ballot,mbox,len);

                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    //@ close mbox_geq_1(ballot, mbox,1);
                    break;
                }
                if (timeout()){
                    //@ close mbox_geq_1(ballot, mbox,0);
                    break;}

            }//end reception loop second round follower

            //@ assert mbox_geq_1(ballot,mbox,len);
            //@ open mbox_geq_1(ballot,mbox,len);

			if (iter == ballot) {
			if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                //@ close mbox_geq_1(ballot,mbox,1);
                //@ assert mbox_geq_1(ballot,mbox,1);
                //@ open mbox_geq_1(ballot,mbox,1);
                //@  old_ballot = ballot;
                leader = mbox->message->leader;
                //@ close mbox_geq_1(old_ballot,mbox,1);
                // close max_tag_eq(ballot,round,mbox);

                // assert max_tag_mbox(ballot,round,mbox);


                //@ close tag_leq(old_ballot,old_round,ballot,round);
                //@assert tag_leq(old_ballot,old_round,ballot,round);
                //@ open tag_leq(old_ballot,old_round,ballot,round);


                // max_tag_mbox_to_list_pred_lemma(mbox);
                //end update of second round leader


                //@ old_round = round;
                round = AckBallot_ROUND;
                //@ close tag_leq(old_ballot,old_round,ballot,round);
                //@assert tag_leq(old_ballot,old_round,ballot,round);
                //@ open tag_leq(old_ballot,old_round,ballot,round);


                m = (msg *) malloc(sizeof(msg));
                if(m==0) {abort();}
                m->ballot = ballot;
                m->round = AckBallot_ROUND;
                m->leader = leader;

                //@assert(m->ballot == ballot && m->round == round);
                send(m, to_all);

                free(m);
                m = NULL;

                list_dispose1(mbox);
                mbox = NULL;

                //@ close mbox_tag_eq(ballot, round,mbox);
                while(true)
                    //@ invariant mbox_tag_eq(ballot, round,mbox);
                {

                    m = recv();
                    if (m != NULL && m->ballot == ballot && m->round == AckBallot_ROUND){
                        //@ open mbox_tag_eq(ballot, round,mbox);
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) {abort();}
                        mbox_new->message =m;
                        if(mbox!=0)
                            {mbox_new->size = mbox->size + 1; }
                        else{mbox_new->size =1 ;}
                        mbox_new->next = mbox;
                        //@ close mbox_tag_eq(ballot, round,mbox);
                        mbox = mbox_new;
                        //@ close mbox_tag_eq(ballot, round,mbox);
                    }
                    else {free(m);}
                    if (timeout()){
                        break;
                    }

                    //@ open mbox_tag_eq(ballot, round,mbox);
                    if(mbox != NULL && mbox->size > n/2){
                        //@ close mbox_tag_eq(ballot, round,mbox);
                        break;
                    }

                    //@ close mbox_tag_eq(ballot, round,mbox);
                }//end reception loop of third round:leader

                //@ assert mbox_tag_eq(ballot, round,mbox);


                if(mbox != NULL && mbox->size > n/2){
                     if (all_same(mbox,leader)==1){
                 //@ assert mbox_tag_eq(ballot,round,mbox);

                   out(ballot, leader);

                }
                }
                //@ close mbox_tag_eq(ballot,round,mbox);
                //@ mbox_tag_eq_to_list_pred_lemma(mbox);

                if(mbox!=0) {list_dispose(mbox);}

                    //@ old_ballot = ballot;
                    ballot++;
                    //@ old_round = round;
                    round = NewBallot_ROUND;

                    //@ close tag_leq(old_ballot,old_round,ballot,round);
                    //@assert tag_leq(old_ballot,old_round,ballot,round);
                    //@ open tag_leq(old_ballot,old_round,ballot,round);


            } else {
                //@close mbox_geq_1(ballot,mbox,len);
                list_dispose1(mbox);
                mbox = NULL;
                //@ old_ballot = ballot;
                ballot++;
                //@ old_round = round;
                round = NewBallot_ROUND;
                //@ close tag_leq(old_ballot,old_round,ballot,round);
                //@assert tag_leq(old_ballot,old_round,ballot,round);
                //@ open tag_leq(old_ballot,old_round,ballot,round);

            }//timeout of the first round go back to the loops begining with new ballot.
		}


        }//end LEADER
        else{
            //FOLLOWER

            //@close  tag_leq(old_ballot,old_round,ballot,round);
            //@assert tag_leq(old_ballot,old_round,ballot,round);
            //@open  tag_leq(old_ballot,old_round,ballot,round);

            round = NewBallot_ROUND;
            //@ old_ballot = ballot;
            //@ old_round = round;




            mbox= NULL;
            reset_timeout();
            //@ int len = 0;
            while(true)
                //@ invariant mbox==NULL && len ==0;
            {

                m = recv();
                if (m != NULL && m->ballot >= ballot && m->round == NewBallot_ROUND){

                    mbox_new = (list*) malloc(sizeof(list));
                    if(mbox_new==0) {
                    abort();
                    }
                    mbox_new->message =m;
                    if(mbox!=0)
                        {mbox_new->size = mbox->size + 1; }
                    else
                    {mbox_new->size =1 ;}
                    mbox_new->next = mbox;
                    //@ len =1;
                    mbox = mbox_new;
                    //@ close mbox_geq_1(ballot,mbox,1);
                } else { //@ close mbox_geq_1(ballot,NULL,0);
                    free(m);}

                //@ open mbox_geq_1(ballot,mbox,len);


                if(mbox != NULL && mbox->size ==1 && mbox->next==NULL){
                    //@ close mbox_geq_1(ballot, mbox,1);
                    break;
                }

                if (timeout()){
                    //@ close mbox_geq_1(ballot, mbox,0);
                    break;}

            }//end reception loop second round follower

            //@ assert mbox_geq_1(ballot,mbox,len);
            //@ open mbox_geq_1(ballot,mbox,len);

			if (iter == ballot){
			if(mbox != NULL && mbox->size ==1&& mbox->next==NULL){
                //@ close mbox_geq_1(ballot,mbox,1);
                //@ assert mbox_geq_1(ballot,mbox,1);
                //@ open mbox_geq_1(ballot,mbox,1);
                //@  old_ballot = ballot;
                leader = mbox->message->leader;
                //@ close mbox_geq_1(old_ballot,mbox,1);
                // close max_tag_eq(ballot,round,mbox);

                // assert max_tag_mbox(ballot,round,mbox);

                //@ close tag_leq(old_ballot,old_round,ballot,round);
                //@assert tag_leq(old_ballot,old_round,ballot,round);
                //@ open tag_leq(old_ballot,old_round,ballot,round);


                //@ old_round = round;
                round = AckBallot_ROUND;
                //@ close tag_leq(old_ballot,old_round,ballot,round);
                //@assert tag_leq(old_ballot,old_round,ballot,round);
                //@ open tag_leq(old_ballot,old_round,ballot,round);

                m = (msg *) malloc(sizeof(msg));
                if(m==0) {
                abort();
                }
                m->ballot = ballot;
                m->round = AckBallot_ROUND;
                m->leader = leader;

                //@assert(m->ballot == ballot && m->round == round);
                send(m, to_all);

                free(m);
                m = NULL;
                list_dispose1(mbox);
                mbox = NULL;

                //@ close mbox_tag_eq(ballot, round,mbox);
                while(true)
                    //@ invariant mbox_tag_eq(ballot, round,mbox);
                {

                    m = recv();
                    if (m != NULL && m->ballot == ballot && m->round == AckBallot_ROUND){
                        //@ open mbox_tag_eq(ballot, round,mbox);
                        mbox_new = (list*) malloc(sizeof(list));
                        if(mbox_new==0) {
                        abort();
                        }
                        mbox_new->message =m;
                        if(mbox!=0)
                            {
                            mbox_new->size = mbox->size + 1;
                            }
                        else  { mbox_new->size =1 ;}

                        mbox_new->next = mbox;
                        //@ close mbox_tag_eq(ballot, round,mbox);
                        mbox = mbox_new;
                        //@ close mbox_tag_eq(ballot, round,mbox);

                    }
                    else {free(m);}
                    if (timeout()){
                        break;
                    }

                    //@ open mbox_tag_eq(ballot, round,mbox);
                    if(mbox != NULL && mbox->size > n/2){

                        //@ close mbox_tag_eq(ballot, round,mbox);
                        break;
                    }


                    //@ close mbox_tag_eq(ballot, round,mbox);
                }//end reception loop of third round:leader

                //@ assert mbox_tag_eq(ballot, round,mbox);

                //@ open  mbox_tag_eq(ballot, round,mbox);
                if(mbox != NULL && mbox->size > n/2){
                    if (all_same(mbox,leader)==1){
                    //@ assert mbox_tag_eq(ballot,round,mbox);
                    // log[ballot] = leader;
                    out(ballot, leader);
                }

                }
                    //@ close mbox_tag_eq(ballot, round,mbox);
                    //@ mbox_tag_eq_to_list_pred_lemma(mbox);

                if(mbox!=0) {list_dispose(mbox);}
                    //@ old_ballot = ballot;
                    ballot++;
                    //@ old_round = round;
                round = NewBallot_ROUND;

                    //@ close tag_leq(old_ballot,old_round,ballot,round);
                    //@assert tag_leq(old_ballot,old_round,ballot,round);
                    //@ open tag_leq(old_ballot,old_round,ballot,round);

        }else {
                //@close mbox_geq_1(ballot,mbox,len);

                list_dispose1(mbox);
                mbox = NULL;
                //@ old_ballot = ballot;
                ballot++;
                //@ old_round = round;
                round = NewBallot_ROUND;
                //@ close tag_leq(old_ballot,old_round,ballot,round);
                //@assert tag_leq(old_ballot,old_round,ballot,round);
                //@open tag_leq(old_ballot,old_round,ballot,round);


            }//timeout of second round and go back to the loops begining into a new ballot

        }//END FOLLOWER
	}

    }//END LOOP LEADER_ELECTION


    return 1;
}

int reset_timeout();
//@ requires emp;
//@ ensures emp;

int coord(int net_size);
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;



int timeout();
//@ requires emp;
//@ ensures emp;

msg* recv();
//@ requires emp;
//@ ensures result->round |-> ?r &*& result->ballot |-> ?v &*& result->leader|->_ &*& malloc_block_Msg(result) &*& INT_MIN <v &*& v < INT_MAX;

void send(msg* message, int pid);
//@ requires true;
//@ ensures true;

int max_log_size(struct List* mbox);
//@requires list_pred(mbox);
//@ensures list_pred(mbox) &*& result>=0;


struct arraylist* longest_log(struct List* mbox, int lastIndex);
//@requires list_pred(mbox);
//@ensures list_pred(mbox);

void list_dispose(struct List *l);
//@ requires list_pred(l);
//@ ensures emp;

/*@
 // Predicate describing the min of a complete linked list
 predicate mbox_tag_eq(int val, int val2, struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->ballot |-> ?v &*& msg->round |-> ?r &*&
  msg->leader |-> _  &*&
 malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val== v &*&  val2== r &*& mbox_tag_eq(val,val2, next) ;
 @*/

/*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void mbox_tag_eq_to_list_pred_lemma(struct List *l)
 requires mbox_tag_eq( ?val, ?val2, l);
 ensures list_pred(l);
 {
 open mbox_tag_eq(val,val2, l);
 if (l != 0) {

 mbox_tag_eq_to_list_pred_lemma(l->next);

 // We need the next two lines to let VeriFast know that l != l->next
 open list_pred(l->next);
 close list_pred(l->next);
 close list_pred(l);
 }
 else close list_pred(0);
 }
 @*/

/*@
 // Predicate describing a complete linked list
 predicate list_pred(struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next
 &*& msg->round |-> ?r &*& msg->ballot |-> _ &*&
  msg->leader |-> _  &*& malloc_block_Msg(msg) &*& list_pred(next);
 @*/
