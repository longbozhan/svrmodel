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

    m_shmSize = shmsize;
    
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

int ShmQueue::getUsedSize()
{
    return (m_ptHeader->iTailPos - m_ptHeader->iHeadPos + m_shmSize) % (m_shmSize);
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
    int freeSize = m_shmSize - getUsedSize();
    if (totalLen > freeSize)
    {
        printf("error, queue is full\n");
        return -1;
    }

    char * tmpBuf = new char[totalLen];
    memset(tmpBuf, 0, sizeof(buf));
    char * p = tmpBuf;
    memcpy(p, &dataHeader, sizeof(DataHead_t));
    p += sizeof(DataHead_t);
    memcpy(p, buf, len);
    p = tmpBuf;
    if (m_shmSize - m_ptHeader->iTailPos < totalLen) 
    {
        int remain = m_shmSize - totalLen;
        memcpy(m_ptHeader->data + m_ptHeader->iTailPos, p, remain);
        p += remain;
        memcpy(m_ptHeader->data, p, totalLen - remain);
        m_ptHeader->iTailPos = totalLen - remain;
    }
    else
    {
        memcpy(m_ptHeader->data + m_ptHeader->iTailPos, p, totalLen);
        m_ptHeader->iTailPos += totalLen;
    }
}

int ShmQueue::atomPop(char ** buf, int &len)
{
    int usedSize = getUsedSize();

    if (usedSize < sizeof(DataHead_t))
    {
        printf("queue empty\n");
        return -1;
    }

    DataHead_t header;
    char * p = (char *)&header;
    if (m_ptHeader->iTailPos < m_ptHeader->iHeadPos) 
    {
        int remain;
        if (m_shmSize - m_ptHeader->iHeadPos < sizeof(DataHead_t)) // head not full
        {
            remain = m_shmSize - m_ptHeader->iHeadPos;
            memcpy(p, m_ptHeader->iHeadPos + m_ptHeader->data, remain);
            p+=remain;
            memcpy(p, m_ptHeader->data, sizeof(DataHead_t) - remain);
            
            len = header.iDataLen;
            *buf = new char[len];
            memcpy(*buf, m_ptHeader->data + sizeof(DataHead_t) - remain, len);
            m_ptHeader->iHeadPos = sizeof(DataHead_t) - remain + len;
        }
        else
        {
            memcpy(p, m_ptHeader->iHeadPos + m_ptHeader->data, sizeof(DataHead_t));
            len = header.iDataLen;
            *buf = new char[len];
            remain = m_shmSize - m_ptHeader->iHeadPos;
            if (remain < len) // data not full
            {
                p = *buf;
                memcpy(p, m_ptHeader->iHeadPos + m_ptHeader->data + sizeof(DataHead_t), remain);
                p += remain;
                memcpy(p, m_ptHeader->data, len - remain);
                m_ptHeader->iHeadPos = len - remain;
            }
            else
            {
                memcpy(p, m_ptHeader->iHeadPos + m_ptHeader->data + sizeof(DataHead_t), len);
                m_ptHeader->iHeadPos += sizeof(DataHead_t) + len;
            }
        }
    }
    else
    {
        memcpy(p, m_ptHeader->data + m_ptHeader->iHeadPos, sizeof(DataHead_t));
        if (header.iDataLen <= 0)
        {
            printf("data empty\n");
            return -2;
        }
        len = header.iDataLen;
        *buf = new char[len];
        memcpy(*buf, m_ptHeader->data + m_ptHeader->iHeadPos + sizeof(header), len);
        m_ptHeader->iHeadPos += sizeof(header) + len;
    }

    return 0;
}

