#ifndef _NOTIFY_H_
#define _NOTIFY_H_
#include "notify.h"
#endif

#ifndef _SHMQUEUE_H_
#define _SHMQUEUE_H_
#include "shmQueue.h"

class Parameter
{
    public:
        Parameter();
        ~Parameter();

        Notify * poNotify;
        ShmQueue * poInQueue;
        ShmQueue * poOutQueue;
        struct event_base *base;
};

#endif
