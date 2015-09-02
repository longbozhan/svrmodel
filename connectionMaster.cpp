#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#include <event.h>
#include<pthread.h>

#include "connectionMaster.h"
#include "Utils.h"

using namespace std;

int g_iListenFd;

ConnectionMaster::ConnectionMaster(Parameter * a_ptPara)
{
    m_ptPara = a_ptPara;
}

ConnectionMaster::~ConnectionMaster()
{
}

// 读事件回调函数 
void onRead(int iCliFd, short iEvent, void *arg) 
{ 
    int iLen; 
    char buf[1024]; 

    cout << "on read" << endl;

    Parameter * para = (Parameter *) arg;
    while(true)
    {
        iLen = recv(iCliFd, buf, sizeof(buf) - 1, 0);
        if (iLen <= 0 && errno == EINTR)
            continue;
        else if (iLen <= 0)
        {
            cout <<"connect close" << endl;
            break;
        }
        buf[iLen] = '\0';
      //  cout <<"recv: " << buf << " len: " << iLen << endl;
        int times = 2;
        int ret = 0;
        while((ret = para->poInQueue->push(buf, iLen))!=0 && times--)
        {
            sleep(1);
        }
        
    }
    
    cout << "Client Close" << endl; 
   
    // 连接结束(=0)或连接错误(<0)，将事件删除并释放内存空间 
    //struct event *pEvRead = (struct event*)arg; 
    //event_del(pEvRead); 
    //delete pEvRead; 

    close(iCliFd); 
    return; 
} 
   
// 连接请求事件回调函数 
void onAccept(int iSvrFd, short iEvent, void *arg) 
{ 
    int iCliFd; 
    struct sockaddr_in sCliAddr;
    Parameter *para= (Parameter *) arg;

    socklen_t iSinSize = sizeof(sCliAddr); 
    iCliFd = accept(iSvrFd, (struct sockaddr*)&sCliAddr, &iSinSize); 
    printf("new accpet\n");
   
    // 连接注册为新事件 (EV_PERSIST为事件触发后不默认删除) 
    struct event *pEvRead = new event; 
    event_set(pEvRead, iCliFd, EV_READ, onRead, para); 
    event_base_set(para->base, pEvRead); 
    event_add(pEvRead, NULL); 
    
    struct event evListen;
    event_set(&evListen, g_iListenFd, EV_READ|EV_PERSIST, onAccept, para);
    event_base_set(para->base, &evListen);
    event_add(&evListen, NULL);
} 

void ConnectionMaster::run()
{
    struct event_base *base = m_ptPara->base;
    
    int iListenFd = Utils::Listen(8888, 5, "127.0.0.1");
    g_iListenFd = iListenFd;

    struct event evListen; 
    // 设置事件 
    event_set(&evListen, iListenFd, EV_READ|EV_PERSIST, onAccept, m_ptPara); 
    // 设置为base事件 
    event_base_set(base, &evListen); 
    // 添加事件 
    event_add(&evListen, NULL); 
    // 事件循环 
    while(true)
    {
        event_base_loop(base, 0);
        printf("...................\n");
    }
}
