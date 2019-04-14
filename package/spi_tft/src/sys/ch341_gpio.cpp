#include "sys.h"

int gpio_fd_dc = -1;
int gpio_fd_bl = -1;

int gpio_init(void)
{
	/*
	if ((gpio_fd_dc = open("/sys/class/gpio/gpio4/value", O_RDWR)) == -1) 
	{
		perror("open gpio4");
		return -1;
	}
	if ((gpio_fd_bl = open("/sys/class/gpio/gpio5/value", O_RDWR)) == -1) 
	{
		perror("open gpio5");
		return -1;
	}
	*/
	FILE* set_export;

    //打开设备节点
	set_export = fopen ("/sys/class/gpio/export", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/export!\n");
	else fprintf(set_export,"41");
	fclose(set_export);
    set_export = fopen ("/sys/class/gpio/export", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/export!\n");
	else fprintf(set_export,"42");
	fclose(set_export);
	//设置成输出
	set_export = fopen ("/sys/class/gpio/gpio41/direction", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/gpio41/direction!\n");
	else fprintf(set_export,"out");
	fclose(set_export);//设置
    set_export = fopen ("/sys/class/gpio/gpio42/direction", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/gpio42/direction!\n");
	else fprintf(set_export,"out");
	fclose(set_export);//设置

	gpio_fd_dc = open ("/sys/class/gpio/gpio41/value", O_RDWR);
    gpio_fd_bl = open ("/sys/class/gpio/gpio42/value", O_RDWR);

	write(gpio_fd_dc,"1",1);
	write(gpio_fd_bl,"1",1);
}

int input() 
{
    
	int fd;
	
	if ((fd = open("/sys/class/gpio/gpio4/value", O_RDWR)) == -1) 
	{
		perror("open");
		return -1;
	}

	struct pollfd fds[1];
	
    fds[0].fd = fd;
	fds[0].events = POLLPRI;

	char buf;

	while (1) 
	{
	    // wait for new GPIO value interrupt
		if (poll(fds, 1, -1) == -1) 
		{
			perror("poll");
			return -1;
		}

        // read one char from GPIO value file
		if (read(fd, &buf, 1) == -1) 
		{
			perror("read");
			return -1;
		}

        // rewind GPIO value file to first char
		if (lseek(fd, 0, SEEK_SET) == -1) {
			perror("lseek");
			return -1;
		}

		printf("new value on pin %s, value = %c\n", "/sys/class/gpio/gpio4/value", buf);
	}

	close(fd);

	return 0;
}

int output(int value) 
{
    
	int fd;
	
	if ((fd = open("/sys/class/gpio/gpio4/value", O_RDWR)) == -1) 
	{
		perror("open");
		return -1;
	}
    

	if (write(fd, value ? "1" : "0", 1) == -1) 
	{
		perror("write");
		return -1;
	}

	close(fd);

	return 0;
}