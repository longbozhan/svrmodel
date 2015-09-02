#include "sem.h"

class ShmQueue
{
    public:
        typedef struct tagDataHead
        {
            int iDataLen;
            int iCheck;
            tagDataHead()
            {
                iDataLen = 0;
                iCheck = 0;
            }
        } DataHead_t;

        typedef struct tagQueueHead
        {
            int iHeadPos;
            int iTailPos;
            int usedSize;
            char data[1];
        } QueueHead_t;

        ShmQueue();
        ~ShmQueue();
        
        int atomPush(char *buf, int n);
        int atomPop(char ** buf, int &n);
        
        int push(char *buf, int n);
        int pop(char ** buf, int &n);
        int init(int, const char *);
        
        int getUsedSize();
        void setUsedSize(int);

    private:
        QueueHead_t * m_ptHeader;
        CSem * m_poSem;
        int m_shmSize;      
        
};
