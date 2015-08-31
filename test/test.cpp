#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char * argv[])
{
    int N = 1024;
    char buf[N];
    int len;
    FILE * fp = fopen(argv[1], "r");
    while((len = fread(buf, sizeof(char), N - 1, fp)))
    {
        buf[len] = '\0';
        printf("size(%d) %s", len, buf);
    }
    fclose(fp);

    return 0;
}
