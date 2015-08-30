#include <stdio.h>

#include "svrkit.h"
#include "Utils.h"

int main()
{
    Utils::signalHandler();
    Utils::folkAsDaemon();
    
    SvrKit server;
    if (server.init() == 0)
    {
        server.run();
    }
    else
    {
        printf("init server error\n");
        return -1;
    }
    
    return 0;
}
