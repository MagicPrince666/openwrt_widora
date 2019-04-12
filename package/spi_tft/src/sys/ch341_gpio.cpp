#include "sys.h"

int gpio_fd_dc = -1;
int gpio_fd_bl = -1;

int gpio_init(void)
{
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