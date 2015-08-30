#include <stdio.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <assert.h>

#include "shmQueue.h"

ShmQueue::ShmQueue()
{

}

ShmQueue::~ShmQueue()
{

}

int ShmQueue::init(int shmkey, const char * lockpath)
{
    printf("in shmQueue init\n");

    int shmsize = 4 * 1024;

    int shmid = 0;
    {
        shmid = shmget ( shmkey, shmsize, 0666 );
        if( shmid<0 )
        {
            shmid = shmget ( shmkey, shmsize, 0666|IPC_CREAT ) ;
        }
        if ( shmid<0 )
        {
            printf("init shmget error(%d) (%s)\n", errno, strerror(errno));
            return -1;
        } 
    }
    m_ptHeader = (QueueHead_t *) shmat ( shmid, NULL, 0 ) ;
    if ( -1 == (long)m_ptHeader )
    {
        printf("shmat error\n");
        return -2;
    }
    m_ptHeader->iHeadPos = m_ptHeader->iTailPos = 0;
    
    // sem lock
    key_t semkey = ftok( lockpath, 0 );
    printf("err(%s) path(%s)\n", strerror(errno), lockpath);
    m_poSem = new CSem(semkey, 1, 0666);
    
    int before=0, after=0;
    {
        before = m_poSem->GetValue(0);
        m_poSem->SetValue(0, 1);
        after = m_poSem->GetValue(0);
    }
    printf( "shmque: init shm queue lockpath %s semkey %i value before %i after %i shmkey %i shmsize %i\n",
            lockpath, semkey, before, after, shmkey, shmsize );

    return 0;
}

int ShmQueue::push(char *buf, int len)
{
    int ret = -1;
    assert ( m_poSem->SemOpWithUndo( 1, 0, -1 ) );
    printf("before push\n");
    ret = atomPush (buf, len);
    printf("after push\n");
    assert ( m_poSem->SemOpWithUndo( 1, 0, 1 ) );
    return ret;
}

int ShmQueue::pop(char ** buf, int &len)
{
    int ret = -1;
    assert ( m_poSem->SemOpWithUndo( 1, 0, -1 ) );
    ret = atomPop(buf, len);
    assert ( m_poSem->SemOpWithUndo( 1, 0, 1 ) );
    return ret;
}

int ShmQueue::atomPush(char *buf, int len)
{
    DataHead_t dataHeader;
    dataHeader.iDataLen = len;
    int totalLen = sizeof(DataHead_t) + len;
    memcpy(m_ptHeader->data + m_ptHeader->iTailPos, &dataHeader, sizeof(DataHead_t));
    memcpy(m_ptHeader->data + sizeof(DataHead_t) + m_ptHeader->iTailPos, buf, len);
    m_ptHeader->iTailPos += totalLen;
}

int ShmQueue::atomPop(char ** buf, int &len)
{
    if (m_ptHeader->iTailPos <= m_ptHeader->iHeadPos)
    {
        printf("queue empty\n");
        return -1;
    }
    
    DataHead_t dataHead;
    memcpy(&dataHead, m_ptHeader->data + m_ptHeader->iHeadPos, sizeof(dataHead));
    if (dataHead.iDataLen <= 0)
    {
        printf("data empty\n");
        return -2;
    }

    *buf = new char[dataHead.iDataLen];
    len = dataHead.iDataLen;
    memcpy(*buf, m_ptHeader->data + m_ptHeader->iHeadPos + sizeof(dataHead), len);
    
    m_ptHeader->iHeadPos += sizeof(dataHead) + len;

    return 0;
}

