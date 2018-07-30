int main()
{
    int commit = 0;
    int count = 1;
    int num_mbox = 0;

    volatile int random;

    int retry;

    int lab = 1;

    int old_count = count - 1;
    int old_lab = 0;

    int m;
    int* mbox[200];

    while (count < 10000) {

        lab = 1;

        assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
        old_count = count;
        old_lab = lab;

        if (pid == leader) {
            send(transaction, all);

            lab = 2;

            assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
            old_count = count;
            old_lab = lab;

            num_mbox = 0;

            retry = random;
            while(retry && num_mbox < num){
                if(filter_cr2(&m)) {
                    mbox[num_mbox] = &m;
                    num_mbox++;
                }

                if(num_mbox == num) {
                    break;
                }

                retry = random;
            }

            if (num_mbox == num) {
                commit = 1;
                lab = 3;

                assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
                old_count = count;
                old_lab = lab;

            }
            else {
                commit = 0;
                lab = 3;

                assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
                old_count = count;
                old_lab = lab;
            }

            lab = 4;

            assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
            old_count = count;
            old_lab = lab;

            num_mbox = 0;

            retry = random;

            while(retry && num_mbox < num){
                if(filter_c2(&m)) {
                    mbox[num_mbox] = &m;
                    num_mbox++;
                }
                if(num_mbox == num) {
                    break;
                }

                retry = random;
            }

            if (num_mbox == num) {
                count = count + 1;
            }
            else {
                count = count + 1;
            }
        }
        else {
            num_mbox = 0;

            retry = random;

            while(retry && num_mbox < 1) {
                if(filter_cr1(&m)) {
                    mbox[num_mbox] = &m;
                    num_mbox++;
                }

                if(num_mbox >= 1) {
                    break;
                }

                retry = random;
            }

            if (num_mbox >= 1) {
                lab = 2;

                assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
                old_count = count;
                old_lab = lab;

                lab = 3;

                assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
                old_count = count;
                old_lab = lab;

                num_mbox = 0;

                retry = random;

                while(retry && num_mbox < 1){
                    if(filter_c1(&m)) {
                        mbox[num_mbox] = &m;
                        num_mbox++;
                    }

                    if(num_mbox >= 1) {
                        break;
                    }

                    retry = random;
                }

                if (num_mbox >= 1) {
                    lab = 4;

                    assert((count > old_count) || ((count == old_count) && (lab > old_lab)));
                    old_count = count;
                    old_lab = lab;

                    count = count + 1;
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
