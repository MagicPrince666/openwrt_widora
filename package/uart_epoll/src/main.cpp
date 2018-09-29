#include <stdio.h>
#include <fcntl.h>      /*file control*/
#include <signal.h>
#include <pthread.h> 
#include <iostream>
#include <unistd.h>

#include "my_socket_c.h"
#include "my_socket_s.h"
#include "com.h"


using namespace std;

Serial serial; 
int run = 1;

static void sigint_handler(int sig)
{
	run = 0;
	serial.run = 0;
    cout << "--- quit the loop! ---" << endl;
}

void * uart_send (void *arg) 
{
    while(run)
    {
        write(serial.fd[0], "hello!,I'm uart1\n",17);	
        sleep(1);
    }
    pthread_exit(NULL);
}

void * uart_rev (void *arg) 
{
	char tmp[128] = {0};
	
	serial.EpollInit(serial.fd);

    while(run)
    {
		bzero(tmp,sizeof(tmp));		
		serial.ComRead(tmp,128);//读取8个字节放到缓存	
	}

	close(serial.epid);
	close(serial.fd[0]);
    pthread_exit(NULL);
}

int main(int argc,char **argv)
{
	
	pthread_t pthread_id[2];//线程ID

	//signal(SIGINT, sigint_handler);//信号处理

	serial.fd[0] = serial.openSerial((char *)"/dev/ttyS1");
	
	if(serial.fd[0] < 0)
	{
		printf("open com1 fail!\n");
		return 0;
	}
	
	tcflush(serial.fd[0],TCIOFLUSH);//清空串口输入输出缓存

	// if (pthread_create(&pthread_id[0], NULL, uart_send, NULL))
	// 	cout << "Create uart_send error!" << endl;
	if (pthread_create(&pthread_id[1], NULL, udp_net, NULL))
		cout << "Create udp_net error!" << endl;    

	// if(pthread_id[0] !=0) {                   
	// 	pthread_join(pthread_id[0],NULL);
	// 	cout << "uart_send "<< pthread_id[0]<< " exit!"  << endl;
	// }
	if(pthread_id[1] !=0) {                   
		pthread_join(pthread_id[1],NULL);
		cout << "udp_net " << pthread_id[1] << " exit!"  << endl;
	}
	
	return 0;
}