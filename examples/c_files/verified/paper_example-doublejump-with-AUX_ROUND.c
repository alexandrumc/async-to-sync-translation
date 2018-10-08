#include "stdlib.h"
#include<stdio.h>
#include<limits.h>

typedef struct Msg {
	int round;
	int epoch;
} msg;


/*@
 predicate msg_pred(struct Msg * msg) =
 msg == 0? true : msg->round |-> _ &*& msg->epoch |-> _ &*& malloc_block_Msg(msg);
 @*/


/*@
 
 predicate msg_val_pred(struct Msg *n, int v1, int v2)
 requires n->round |-> v1 &*& n->epoch |-> v2 &*& malloc_block_Msg(n);
 
 @*/


typedef struct List{
    msg * message;
    struct List * next;
    int size;
} list;

/*@
 // Predicate describing a complete linked list
 predicate list_pred(struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& n->size |-> ?size &*& n->next |-> ?next &*& malloc_block_List(n) &*& n!=next 
 &*& msg->round |-> _ &*& msg->epoch |-> _ &*& malloc_block_Msg(msg) &*& list_pred(next);
 @*/


/*@
 // Predicate describing a segment of a linked list from 'start' until
 // 'end' (not including 'end')
 predicate lseg_pred(struct List* start, struct List* end;) =
 start == end ?
 true
 :
 start->message |-> ?msg &*& start->size |-> ?size &*&  start->next |-> ?next &*& start != 0 &*& start != next 
 &*& malloc_block_List(start)&*& msg->round |-> _ &*& msg->epoch |-> _ &*& malloc_block_Msg(msg)  &*& lseg_pred(next, end)  ;
 @*/



/*@
 // Lemma for converting a full list predicate into a list segment predicate
 lemma void list_to_lseg(struct List *l)
 requires list_pred(l);
 ensures lseg_pred(l, 0);
 {
 open list_pred(l);
 if (l != 0) {
 list_to_lseg(l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open lseg_pred(l->next, 0);
 close lseg_pred(l->next, 0);
 }
 close lseg_pred(l, 0);
 }
 @*/

/*@
 // Lemma for converting a list segment predicate describing a complete
 // list into a full list predicate.
 lemma void lseg_to_list(struct List *n)
 requires lseg_pred(n, 0);
 ensures list_pred(n);
 {
 open lseg_pred(n, 0);
 if (n != 0) {
 lseg_to_list(n->next);
 }
 close list_pred(n);
 }
 @*/






 

/*@
 // Lemma for converting a full min list predicate into a lseg predicate
 lemma void min_list_to_lseg_lemma(int val, int rval, struct List *l)
 requires min_list_pred(val, rval, l);
 ensures lseg_pred(l,0);
 {
 open min_list_pred(val,rval, l);
 if (l != 0) {
 min_list_to_lseg_lemma(val,rval, l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open lseg_pred(l->next,0);
 close lseg_pred(l->next,0);
 }
 close lseg_pred(l,0);
 }
 @*/

/*@
 // Lemma for converting a full min list predicate into a list predicate
 lemma void min_list_to_list_pred_lemma(int val, int rval, struct List *l)
 requires min_list_pred(val,rval, l);
 ensures list_pred(l);
 {
 open min_list_pred(val,rval, l);
 if (l != 0) {
 min_list_to_list_pred_lemma(val,rval,l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open list_pred(l->next);
 close list_pred(l->next);
 }
 close list_pred(l);
 }
 @*/



 
/*@
 // Predicate describing the max of a complete linked list
 predicate max_list_pred(int val, struct List* n) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?e &*& msg->round |-> _ &*& malloc_block_Msg(msg) &*& malloc_block_List(n)
 &*& n->next |-> ?next &*& n!= next &*& val>= e &*& n->size |-> ?size &*& max_list_pred(?val1, next) &*& val1 <= val;
 @*/
 
 /*@
 
 // Predicate describing the max of a incomplete linked list
 predicate max_lseg_pred(int val, struct List* start, struct List* end) =
 (start == end )?
 true
 :
  start->message |-> ?msg &*& start->size |-> ?size &*&  start->next |-> ?next &*& start != 0 &*& start != next
  &*& malloc_block_List(start)&*& msg->round |-> ?r &*& msg->epoch |-> ?e &*& malloc_block_Msg(msg)  
  &*& val>= e &*& max_lseg_pred(?val1,next,end) &*& val1 <= val;
 @*/

 
 /*@
 
 // Lemma for adding a node to a max value list segment predicate
 lemma void max_lseg_add_node(struct List* lseg_start)
 requires max_lseg_pred(?val,lseg_start, ?lseg_end) &*& lseg_end != 0 &*&
          lseg_end->message |-> ?msg &*& lseg_end->size |-> _ &*& lseg_end->next |-> ?next  &*&
          malloc_block_List(lseg_end) &*& msg->round |-> ?r &*& msg->epoch |-> ?e &*&
          malloc_block_Msg(msg)  &*& lseg_pred(next, 0);
 ensures max_lseg_pred(?valnew, lseg_start, next) &*& lseg_pred(next, 0) &*& 
         (val >= e ? valnew == val : valnew == e );
 {
    open max_lseg_pred(val, lseg_start, lseg_end);
    if (lseg_start == lseg_end) {
         close max_lseg_pred(INT_MIN,next, next);
    } else {
    max_lseg_add_node(lseg_start->next);
    }
  open lseg_pred(next,0);
  close lseg_pred(next,0);
  if(e <= val)  close max_lseg_pred(val,lseg_start, next);
  else close max_lseg_pred(e,lseg_start, next);
}
 @*/

 /*@
lemma void list_to_max_lseg_lemma(struct List *first)
  requires list_pred(first);
ensures max_lseg_pred(INT_MAX, first, 0);
{
    open list_pred(first);
    if (first != 0) {
        list_to_max_lseg_lemma(first->next);
    }
    close max_lseg_pred(INT_MAX,first, 0); 
   }
 
@*/

 /*@
lemma void max_lseg_to_lseg_lemma(struct List *first,struct List *end)
  requires max_lseg_pred(?val, first, end);
  ensures lseg_pred(first,end);

{
    open max_lseg_pred(val, first, end);
    if (first != end) {
        max_lseg_to_lseg_lemma(first->next,end);
    }
    close lseg_pred(first, end); 
   }
 
@*/


/*@
 lemma void max_lseg_to_max_list_lemma(struct List *n)
 requires max_lseg_pred(?val, n, 0);
 ensures max_list_pred(val,n);
 
 {
 open max_lseg_pred(val, n, 0);
 if (n != 0) {
    max_lseg_to_max_list_lemma(n->next);
 }
 close max_list_pred(val,n);
 }
 
 @*/

/*@
 // Predicate describing the max of a complete linked list
 predicate max_min_list_pred(int mval, int val, int rval, struct List* n) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?e &*& msg->round |-> ?r &*& malloc_block_Msg(msg) &*& malloc_block_List(n)
 &*& n->next |-> ?next &*& n!= next &*& val>= e &*& mval<= e &*& rval ==r &*& n->size |-> ?size &*&
 max_min_list_pred(mval,?val1, rval, next) &*& val1 <= val;
 @*/

/*@
 
 // Predicate describing the max of a incomplete linked list
 predicate max_min_lseg_pred(int mval,int val, int rval, struct List* start, struct List* end) =
 (start == end )?
 true
 :
 start->message |-> ?msg &*& start->size |-> ?size &*&  start->next |-> ?next &*& start != 0 &*& start != next
 &*& malloc_block_List(start)&*& msg->round |-> ?r &*& msg->epoch |-> ?e &*& malloc_block_Msg(msg)
 &*& val>= e &*& e>= mval&*& rval ==r &*& max_min_lseg_pred(mval,?val1,rval,next,end) &*& val1 <= val;
 @*/





/*@
 
 // Lemma for adding a node to a max value list segment predicate
 lemma void max_min_lseg_add_node(int mval, int rval, struct List* lseg_start)
 requires max_min_lseg_pred(mval,?val,rval, lseg_start, ?lseg_end) &*& lseg_end != 0 &*&
 lseg_end->message |-> ?msg &*& lseg_end->size |-> _ &*& lseg_end->next |-> ?next  &*&
 malloc_block_List(lseg_end) &*& msg->round |-> ?r &*& msg->epoch |-> ?e &*& e>=mval &*& rval == r &*& 
 malloc_block_Msg(msg)  &*& min_lseg_pred(mval,rval, next, 0);
 ensures max_min_lseg_pred(mval,?valnew, rval, lseg_start, next) &*& min_lseg_pred(mval, rval, next, 0) &*&
 (val >= e ? valnew == val : valnew == e );
 {
 open max_min_lseg_pred(mval, val,rval, lseg_start, lseg_end);
 if (lseg_start == lseg_end) {
 close max_min_lseg_pred(mval, INT_MIN, rval, next, next);
 } else {
 max_min_lseg_add_node(mval,rval, lseg_start->next);
 }
 open min_lseg_pred(mval, rval, next,0);
 close min_lseg_pred(mval,rval, next,0);
 if(e <= val)  close max_min_lseg_pred(mval, val,rval, lseg_start, next);
 else close max_min_lseg_pred(mval, e,rval, lseg_start, next);
 }
 @*/

/*@
 lemma void min_list_to_max_min_lseg_lemma(int min, int rval, struct List *first)
 requires min_list_pred(min,rval, first);
 ensures max_min_lseg_pred(min,INT_MAX,rval, first, 0);
 {
 open min_list_pred(min,rval, first);
 if (first != 0) {
 min_list_to_max_min_lseg_lemma(min,rval, first->next);
 close max_min_lseg_pred(min,INT_MAX,rval, first, 0);
 }
 else close max_min_lseg_pred(min,INT_MAX,rval, 0, 0);
 }
 
 @*/

/*@
 lemma void max_min_lseg_to_min_lseg_lemma(int mval, int rval, struct List *first,struct List *end)
 requires max_min_lseg_pred(mval,?val, rval, first, end);
 ensures min_lseg_pred(mval,rval, first,end);
 
 {
 open max_min_lseg_pred(mval,val,rval, first, end);
 if (first != end) {
 max_min_lseg_to_min_lseg_lemma(mval,rval,first->next,end);
 }
 close min_lseg_pred(mval,rval,first, end);
 }
 
 @*/

/*@
 // Predicate describing a segment of a linked list from 'start' until
 // 'end' (not including 'end')
 predicate min_lseg_pred(int mval, int rval, struct List* start, struct List* end;) =
 start == end ?
 true
 :
 start->message |-> ?msg &*& start->size |-> ?size &*&  start->next |-> ?next &*& start != 0 &*& start != next
 &*& malloc_block_List(start)&*& msg->round |-> ?r &*& msg->epoch |-> ?e &*& malloc_block_Msg(msg)  &*& mval<=e &*& rval == r &*& min_lseg_pred(mval, rval, next, end)  ;
 @*/

/*@
 // Predicate describing the min of a complete linked list
 predicate min_list_pred(int val, int rval, struct List* n;) =
 n == 0 ?
 true
 :
 n->message |-> ?msg &*& msg->epoch |-> ?v &*& msg->round |-> ?r &*& malloc_block_Msg(msg) &*& malloc_block_List(n) &*& n->next |-> ?next &*& n->size |-> ?s &*&
 n!=next &*&  val<= v &*& rval==r &*&  min_list_pred(val,rval,next);
 @*/

/*@
 lemma void max_min_lseg_to_max_min_list_lemma(int mval, int rval, struct List *n)
 requires max_min_lseg_pred(mval, ?val,rval, n, 0);
 ensures max_min_list_pred(mval, val, rval,n);
 
 {
 open max_min_lseg_pred(mval,val, rval, n, 0);
 if (n != 0) {
 max_min_lseg_to_max_min_list_lemma(mval,rval, n->next);
 }
 close max_min_list_pred(mval,val,rval, n);
 }
 
 @*/




/*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void max_min_list_pred_to_list_pred_lemma(struct List *l)
 requires max_min_list_pred(?mval, ?val,?rval, l);
 ensures list_pred(l);
 {
 open max_min_list_pred(mval, val,rval,l);
 if (l != 0) {
 
    max_min_list_pred_to_list_pred_lemma(l->next);
 
	 // We need the next two lines to let VeriFast know that l != l->next
 	open list_pred(l->next);
 	close list_pred(l->next);
 	close list_pred(l);
 	}
 else close list_pred(0);
 }
 @*/

/*@
 // Lemma for converting a full max list predicate into a list predicate
 lemma void min_list_pred_to_list_pred_lemma(struct List *l)
 requires min_list_pred( ?val,?rval,l);
 ensures list_pred(l);
 {
 open min_list_pred(val,rval,l);
 if (l != 0) {
 
 min_list_pred_to_list_pred_lemma(l->next);
 
 // We need the next two lines to let VeriFast know that l != l->next
 open list_pred(l->next);
 close list_pred(l->next);
 close list_pred(l);
 }
 else close list_pred(0);
 }
 @*/

int max_epoch(struct List* mbox)
//@ requires min_list_pred(?min,?rval, mbox) &*& mbox!=0;
////@ ensures true;
////@ ensures max_lseg_pred(result,mbox,0);
//@ ensures max_min_list_pred(min,result,rval, mbox) &*& mbox!=0;
// max_lseg_pred(result,mbox,0);
{
    
    struct List *current = mbox;
    //@min_list_to_max_min_lseg_lemma(min,rval,mbox);
    //@open max_min_lseg_pred(min,INT_MAX,rval,mbox,0);
    int max = mbox->message->epoch;
    current = mbox->next;
    //@close max_min_lseg_pred(min,max,rval,current,current);
    //@close max_min_lseg_pred(min,max,rval,mbox,current);
    //@ max_min_lseg_to_min_lseg_lemma(min,rval,current,0);
    //@ assert (max_min_lseg_pred(min,max,rval,mbox,current) &*& min_lseg_pred(min,rval,current,0));
    while (current != NULL)
        //@ invariant (max_min_lseg_pred(min,max,rval,mbox,current) &*& min_lseg_pred(min,rval,current,0));
    {
        //@ open min_lseg_pred(min,rval,current,0);
        if(current->message->epoch > max)
            max = current->message->epoch;
        current = current->next;
        //@ max_min_lseg_add_node(min,rval,mbox);
    }
    //@ assert max_min_lseg_pred(min,max,rval,mbox,0);
    //@ max_min_lseg_to_max_min_list_lemma(min,rval,mbox);
    //@assert max_min_list_pred(min,max,rval,mbox);
    return max;
}

bool timeout();
//@ requires emp;
//@ ensures emp;
//{
//    return rand()%3;
//}

enum round_typ {FIRST_ROUND, SECOND_ROUND, AUX_ROUND} ;

msg* recv()
//@ requires emp;
//@ ensures result->round |-> ?v1 &*& result->epoch |-> ?v &*& malloc_block_Msg(result) &*& INT_MIN <v &*& v < INT_MAX;
{
    msg* m = (msg *)malloc(sizeof(msg));
    if(m == 0) abort();
    //@ assume(m->epoch > INT_MIN && m->epoch < INT_MAX);
    return m;
}


void dispose(msg* c)
//@ requires c->epoch |-> _ &*& c->round |-> _ &*& malloc_block_Msg(c);
//@ ensures emp;
{
    free(c);
}


void list_dispose(struct List *l)
//@ requires list_pred(l);
//@ ensures emp;
{
    //@ open list_pred(l);
    struct List* current = l;
    while(current != 0)
        //@ invariant list_pred(current);
    {
        //@ open list_pred(current);
        struct List* oldcurrent = current;
        msg *m = current->message;
        current = current->next;
        free(m);
        free(oldcurrent);
    }
}



void dispose_list(list* c)
//@ requires  c->message |-> _ &*& c->next |-> _ &*& c->size |-> _ &*& malloc_block_List(c);
//@ ensures emp;
{
    free(c);
}


int leader(int phase, int net_size)
//@ requires emp;
//@ ensures 0<=result &*& result < net_size;
{
    int res = phase % net_size;
    //@ assume(0<=res && res<net_size);
    return res;
}

bool reset_timeout();
//@ requires emp;
//@ ensures emp;

void out(int v1, int v2);
//@ requires true;
//@ ensures true;


void send(msg* message, int pid);
//@ requires true;
//@ ensures true;

int main(int argc, char **argv)//@ : main
//@ requires true;
//@ ensures true;
{
    
    int n = argc;
//    int *log;
//    log = malloc(9*sizeof(int));
   
    enum round_typ round;
    int epoch, old_epoch;
    int pid;
    list *mbox = NULL;
    list* mbox_new = NULL;
    msg* m = NULL;
    epoch = 0;
    old_epoch = epoch-1;

    int to_all = n+1;
    
    round = AUX_ROUND;
    enum round_typ old_round = round;
    while(1)
        //@ invariant ((old_epoch<epoch && round == AUX_ROUND));
    {
        old_epoch = epoch;
        round = FIRST_ROUND;
        old_round = round;
        
        //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));

        if (pid == leader(epoch,n))
        {
            m = (msg *) malloc(sizeof(msg));
            if(m==0) abort();
            m->epoch = epoch;
            m->round = FIRST_ROUND;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, to_all);
            
            dispose(m);
            m = NULL;
        }
        reset_timeout();
        mbox = NULL;
        while(true)
            //@ invariant min_list_pred(epoch,round,mbox);
        {
            //@open min_list_pred(epoch,round,mbox);
            m = recv();
            if (m != NULL && m->epoch >= epoch && m->round == FIRST_ROUND){
                
                mbox_new = (list*) malloc(sizeof(list));
                if(mbox_new==0) abort();
                mbox_new->message =m;
                if(mbox!=0)
                    mbox_new->size = mbox->size + 1;
                else  mbox_new->size =1 ;
                mbox_new->next = mbox;
                mbox = mbox_new;
               
            }
            else free(m);
            if (timeout()) break;
                else if(mbox != NULL && mbox->size ==1){
                break;
            }
            //@ close min_list_pred(epoch,round,mbox);
        }
        //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));
        if(mbox!=NULL){
            //@ open (min_list_pred(epoch,round,mbox));
            old_epoch = epoch;
            epoch = mbox->message->epoch;
            round = mbox->message->round;
            //@ assert ((old_epoch<epoch && old_round ==round) || (old_epoch==epoch && old_round ==round));
            
            struct List* next = mbox->next;
       
       	    //@ min_list_to_list_pred_lemma(old_epoch, round,mbox);
       	    old_epoch = epoch;
            
            old_round = round;
            round = SECOND_ROUND;
            
            //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));
            m = (msg *) malloc(sizeof(msg));
            if (m==0) abort();
            m->epoch = epoch;
            m->round = SECOND_ROUND;
            
            //@assert(m->epoch == epoch && m->round == round);
            send(m, to_all);
            dispose(m);
            m = NULL;
            
            list_dispose(mbox);
            reset_timeout();
            mbox = NULL;
            while(true)
            //@ invariant min_list_pred(epoch,round,mbox);
            {
                //@ open min_list_pred(epoch,round,mbox);
                 m = recv();
                 if (m != NULL && m->epoch >= epoch && m->round == SECOND_ROUND){
            
                                mbox_new = (list*) malloc(sizeof(list));
                                if(mbox_new==0) abort();
                                mbox_new->message =m;
                                if(mbox!=0)
                                    mbox_new->size = mbox->size + 1;
                                else  mbox_new->size =1 ;
                                mbox_new->next = mbox;
                                mbox = mbox_new;
            
                    }
                    else free(m);
                    if (timeout()) break;
                    else if(mbox != NULL && mbox->size > n/2){
                                break;
                    }

                    //@ close min_list_pred(epoch,round,mbox);
            }
            
            
     
           
            if (mbox != NULL && mbox->size > n/2){
            	     //@ close min_list_pred(epoch,round,mbox);
                    epoch = max_epoch(mbox);
                
                    //@open  max_min_list_pred(old_epoch,epoch,round,mbox);
                                        
                    //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));

                    //@close  max_min_list_pred(old_epoch,epoch,round,mbox);
                    printf("\n%d", epoch);
                    int leader = leader(epoch,n);
                            //log[epoch] = leader();
                    out(epoch, leader);
        	 //@ max_min_list_pred_to_list_pred_lemma(mbox); 
        	  epoch++;
        	  old_round = round;
       		  round = AUX_ROUND;
       		  //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));

            }
            else{
             //@ close min_list_pred(old_epoch,round,mbox);
             //@ min_list_pred_to_list_pred_lemma(mbox);
            old_epoch = epoch; 
            epoch++;
            old_round = round;
       	    round = AUX_ROUND;
       	    //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));

            }
            
            if(mbox!=0) list_dispose(mbox);
        }else{
        
        old_epoch = epoch; 
        epoch++;
        old_round = round;
        round = AUX_ROUND;
        //@ assert ((old_epoch<epoch) || (old_epoch==epoch && old_round <=round));

        }
    }
    return 1;
}
