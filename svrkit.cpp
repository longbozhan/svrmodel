#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <event.h>

#include "svrkit.h"

SvrKit::SvrKit()
{
    m_ptPara = new Parameter();
}

SvrKit::~SvrKit()
{
    delete m_ptPara;
    m_ptPara = NULL;
}

int SvrKit::init()
{
    int iRet = 0;
    m_ptPara->poNotify = new Notify();
    iRet = m_ptPara->poNotify->init();
    if (iRet < 0)
    {
        printf("init notify error\n");
        return -1;
    }

    m_ptPara->poInQueue = new ShmQueue();
    iRet = m_ptPara->poInQueue->init(12345, "tmp/inqueue.lock");
    if (iRet < 0)
    {
        printf("init inqueue error\n");
        return -2;
    }

    m_ptPara->poOutQueue = new ShmQueue();
    iRet = m_ptPara->poOutQueue->init(12346, "tmp/outqueue.lock");
    if (iRet < 0)
    {
        printf("init outqueue error\n");
        return -3;
    }

    m_ptPara->base = event_base_new(); 


    // just for test
    int pid = fork();
    if (pid == 0)
    {
        exit(0);
        while(1)
        {
            printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
            char * buf;
            int n = 0;
            int ret = m_ptPara->poInQueue->pop(&buf, n);
            printf("ret:%d n:%d\n", ret, n);
            if (ret == 0 && n > 0)
            {
                printf("===================================pop size(%d)\n", n);
                //fwrite(buf, sizeof(char), n, fp);
                //fclose(fp);
                //fp = fopen("file.txt", "a+");
            }
            sleep(1);
        }
    }

    return iRet;
}

void SvrKit::run()
{
    processLayer();
    connectionLayer();
}

void SvrKit::processLayer()
{
    ProcessMaster process(m_ptPara);
    process.run();
}

void SvrKit::connectionLayer()
{
    ConnectionMaster connection(m_ptPara);
    connection.run();
}

