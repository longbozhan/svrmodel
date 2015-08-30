#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "processMaster.h"

ProcessMaster::ProcessMaster(Parameter * a_ptPara)
{
    m_ptPara = a_ptPara;
    m_mapPidInfo.clear();
}

ProcessMaster::~ProcessMaster()
{
}

void ProcessMaster::run()
{
    int iRet = fork();
    if (iRet < 0)
    {
        printf("error fork in processmaster run\n");
        return;
    }
    else if (iRet > 0)
    {
        printf("in parament, continue...\n");
        return;
    }
    
    forkChildRun(5);

    while(true)
    {
        int tPId;
        printf("map size:%d\n", m_mapPidInfo.size());
        while ((tPId = waitpid(-1, NULL, WNOHANG)) > 0) 
        {
            if (m_mapPidInfo.find(tPId) != m_mapPidInfo.end())
            {
                printf("pid(%d) exit\n", tPId);
                m_mapPidInfo.erase(tPId);
                forkChildRun(1);
            }
        }
        printf("in process master...\n");
        sleep(1);
        //int pid = waitpid(-1, NOWHANG);
    }
}

void ProcessMaster::forkChildRun(int iChildNum)
{
    for(int i = 0; i < iChildNum; i++)
    {
        int pid = fork();
        if (pid < 0)
        {
            printf("fork child error\n");
            exit(-1);
        }
        else if (0 < pid)
        {
            printf("in parant, continue\n");
            m_mapPidInfo.insert(make_pair<int,int>(pid, 0));
            continue;
        }

        try
        {
            childRun();
        }
        catch (...)
        {
            printf("catch exception from child run\n");
        }
    }
}

void ProcessMaster::childRun()
{
    printf("a new child...\n");
    while(true)
    {
        char *buf = NULL;
        int ret = 0;
        int n;
        while((ret = m_ptPara->poInQueue->pop(&buf, n)) != 0)
        {
            printf("pid(%d) sleep...\n", getpid());
            sleep(1);
        }
        char * newBuf = new char[n + 1];
        memcpy(newBuf, buf, n);
        newBuf[n] = '\0';
        printf("pid(%d) get buf(%s) len(%d)\n", getpid(), newBuf, n);
        fflush(stdout);
        delete buf;
        buf = NULL;
        delete newBuf;
        newBuf = NULL;
    }
}
