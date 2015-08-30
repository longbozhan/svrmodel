#include <stdlib.h>

namespace Utils
{
    void signalHandler();
    void folkAsDaemon();
    int  Listen ( const unsigned short port, const int queueconn, const char *bindip);
};
