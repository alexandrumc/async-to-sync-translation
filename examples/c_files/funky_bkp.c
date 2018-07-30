const int FIRST_ROUND = 1;
const int LAST_ROUND = 4;

typedef struct Mbox
{
    int messages[100];
    int num_msg;
} Mbox;



int main()
{
    Mbox mbox;
    mbox.num_msg = 0;
    while(1)
    {
        lab = FIRST_ROUND;
        sendto(broadcast);
        while(1)
        {
            m = recv();
            if (filter(m))
            {
                mbox.messages[num_msg] = m;
                mbox.num_msg++;
            }
            if (timeout() || mbox.num_msg > n/2)
                break;
        }

        lab = 2;
        sendto(leader);
        while(1)
        {
            m = recv();
            if (filter_condition_for_message)
                mbox = mbox + m;
            if (timeout() || mbox > n/2)
                break;
        }

        lab = 3;
        sendto(broadcast);
        while(1)
        {
            m = recv();
            if (filter_condition_for_message)
                mbox = mbox + m;
            if (timeout() || mbox > n/2)
                break;
        }

        if (smth)
        {
            i++;
        }
        else
        {
            i--;
        }
        lab = LAST_ROUND;
    }
}



