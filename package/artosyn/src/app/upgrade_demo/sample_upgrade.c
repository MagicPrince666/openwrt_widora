#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include "libar8020.h"

int main(int argc, char *argv[])
{
    PORT port0;

    Cmd_Port_Open(&port0,NULL);
    Cmd_Upgrade(port0,1,argv[1]);
    usleep(100000);
    Cmd_Port_Close(port0);
   
    return 0;

}
