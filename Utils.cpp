#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "Utils.h"

void Utils::folkAsDaemon()
{
    int pid = fork();
    if (pid > 0)
    {
        exit(0);
    }
    else if (pid == 0)
    {
        setsid();
        pid = fork();
        if (pid < 0)
        {
            printf("fork child2 error\n");
        }
        else if (pid > 0)
        {
            exit(0);
        }
        else
        {
            printf("deamon process run...\n");
        }
    }
    else
    {
        printf("fork child error\n");
    }
}

void Utils::signalHandler()
{
    assert ( signal ( SIGPIPE, SIG_IGN ) != SIG_ERR );
    assert ( signal ( SIGALRM, SIG_IGN ) != SIG_ERR );
    assert ( signal ( SIGCHLD, SIG_IGN ) != SIG_ERR );
}

int Utils :: Listen ( const unsigned short port, const int queueconn, const char *bindip)
{
	int listenfd = -1;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0)
	{
		return(-1);
	}

	struct sockaddr_in sin;
	memset((char *) &sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = PF_INET;
	if ( (bindip == NULL) || ('\0' == *bindip) )
	{
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		sin.sin_addr.s_addr = inet_addr(bindip);
	}
	sin.sin_port = htons(port);

	/* set socket option */
	int option = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(int));

    if(bind(listenfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) 
	{
		close ( listenfd );
        return(-1);
    }

    if(listen(listenfd, queueconn) < 0) 
	{
		close ( listenfd );
		return(-1);
	};

	return(listenfd);
}

