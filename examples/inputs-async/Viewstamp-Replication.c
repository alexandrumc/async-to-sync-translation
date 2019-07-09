#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1

enum NormalOperations {Prepare_ROUND, PrepareOk_ROUND};

enum ViewChange {DoViewChange_ROUND, StartView_ROUND};

enum Recovery {Recovery_ROUND, RecoveryResponse_ROUND};

enum Status {normal, view_change, recovering};

typedef struct {
    /* Consider piggy backed in message */
    char *message;
    int request_nr, view_nr;
    int replica_id;
} msg_NormalOp;

typedef struct {
    int aux;
} msg_ViewChange;

typedef struct {
    int aux;
} msg_Recovery;

typedef struct lista {
    int size;
    msg_NormalOp *info;
    struct lista *next;
} listA;

typedef struct listb {
    int size;
    msg_ViewChange *info;
    struct listb *next;
} listB;

typedef struct listc {
    int size;
    msg_Recovery *info;
    struct listb *next;
} listC;

int get_primary(int view_nr) {
    return 0;
}

int main(int argc, char **argv)
{
    /* Check for IP and replica_id as command line args */
    if (argc < 3) {
        printf("Not enough arguments!\n");
        return 0;
    }

    enum Status status = normal;
    enum NormalOperations round = Prepare_ROUND;

    /* view_nr - current view, op_number - most recent received request */
    int view_nr = 0, op_number = 0;

    /* pid it is actually replica id */
    int pid = atoi(argv[1]);

    /* Start normal operation */
    while (true) {
        round = Prepare_ROUND;
        if (pid == get_primary(view_nr)) {

        } else {

        }
    }
    return 0;
}
