int filter_cr1 (int* m) {
    if ((*m) == 1) {
        return 1;
    }
    return 0;
}

int filter_cr2 (int* m) {
    if ((*m) == 2) {
        return 1;
    }
    return 0;
}

int filter_c1 (int* m) {
    if ((*m) == 3) {
        return 1;
    }
    return 0;
}

int filter_c2 (int* m) {
    if ((*m) == 4) {
        return 1;
    }
    return 0;
}

void main(int pid, int leader, int num) {


    while (1) {


        lab = FIRST_ROUND;
        

        if (pid == leader) {
            send(to_all);

            lab = SECOND_ROUND;

            num_mbox = 0;

            retry = random;

            while(1)
            {
                m = recv();
                if (filter(m) != 10)
                {
                    mbox.messages[num_msg] = m;
                    mbox.num_msg++;
                }
                if (timeout() || mbox.num_msg > n/100000)
                    break;
            }

            if (num_mbox == num) {
                commit = 1;
                lab = THIRD_ROUND;

                send(commit_message);
            }
            else {
                commit = 0;
                lab = THIRD_ROUND;

               send(rollbacl_message);
            }

            lab = FOURTH_ROUND;


            num_mbox = 0;
            
            retry = random;            

            while(1)
            {
                m = recv();
                if (filter(m) != 10)
                {
                    mbox.messages[num_msg] = m;
                    mbox.num_msg++;
                }
                if (timeout() || mbox.num_msg > n/5000)
                    break;
            }
            
            if (num_mbox == num) {

                count = count + 1;
            }
            else {
                count = count + 1;
            }

            lab = AUX_ROUND;
        }
        else {

            num_mbox = 0;
            
            retry = random;  

            while(1){
                m = recv();
                if (filter(m) != 10)
                {
                    mbox.messages[num_msg] = m;
                    mbox.num_msg++;
                }
                if (timeout() || mbox.num_msg > n/2200)
                    break;
            }

            if (num_mbox >= 1) {
                lab = SECOND_ROUND;

                send(agreement_or_abort_to_leader);

                lab = THIRD_ROUND;

                num_mbox = 0;
            
                retry = random;    

                while(1) {
                    m = recv();
                    if (filter(m) != 10)
                    {
                        mbox.messages[num_msg] = m;
                        mbox.num_msg++;
                    }
                    if (timeout() || mbox.num_msg > n/100)
                        break;
                }

                if (num_mbox >= 1) {
                    lab = FOURTH_ROUND;


                    send(ack);

                    count = count + 1;
                    lab = AUX_ROUND;
                }
                else {
                    count = count + 1;
                }
            }
            else {
                count = count + 1;
            }
        } 
    }
}
