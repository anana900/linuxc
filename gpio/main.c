#include <stdio.h>
#include <linux/gpio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>

#define PORT_OUT "/dev/gpiochip0"

struct gpiochip_info info;
struct gpioline_info line_info;

int get_info()
{
	int fd, ret;

        fd = open(PORT_OUT, O_RDONLY);

        if (fd < 0)
        {
                printf("Unabled to open %s: %s", PORT_OUT, strerror(errno));
                return 0;
        }

	ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
	if(ret == -1)
	{
		printf("dupa");
		return -1;
	}
	printf("INFO: %s \n%s \n%d\n", info.name, info.label, info.lines);
	close(fd);

	return(0);
}

int get_line_info(int number_of_gpio_lines)
{
        int fd, ret;

        fd = open(PORT_OUT, O_RDONLY);

        if (fd < 0)
        {
                printf("Unabled to open %s: %s", PORT_OUT, strerror(errno));
                return 0;
        }

	//int fd, ret;
	for (int i = 0; i < number_of_gpio_lines; i++)
	{
    		line_info.line_offset = i;
    		ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &line_info);
    		if (ret == -1)
    		{
        		printf("Unable to get line info from offset %d: %s", i, strerror(errno));
    		}
	    	else
    		{
        		printf("offset: %d, name: %s, consumer: %s. Flags:\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\n",
               		i,
               		line_info.name,
               		line_info.consumer,
               		(line_info.flags & GPIOLINE_FLAG_IS_OUT) ? "OUTPUT" : "INPUT",
               		(line_info.flags & GPIOLINE_FLAG_ACTIVE_LOW) ? "ACTIVE_LOW" : "ACTIVE_HIGHT",
               		(line_info.flags & GPIOLINE_FLAG_OPEN_DRAIN) ? "OPEN_DRAIN" : "...",
               		(line_info.flags & GPIOLINE_FLAG_OPEN_SOURCE) ? "OPENSOURCE" : "...",
               		(line_info.flags & GPIOLINE_FLAG_KERNEL) ? "KERNEL" : "");
    		}
	}

	close(fd);
	return(0);
}

int set_gpio(int pin, uint8_t value)
{
        int fd, ret;
        fd = open(PORT_OUT, O_RDONLY);

        struct gpiohandle_request rq;
        rq.lineoffsets[0] = pin;
        rq.lines = 1;
        rq.flags = GPIOHANDLE_REQUEST_OUTPUT;

	struct gpiohandle_data data;

        if (fd < 0)
        {
                printf("Error dostep  GPIO %s: %s", PORT_OUT, strerror(errno));
                return(-1);;
        }

	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
	close(fd);

	if (ret == -1)
	{
    		printf("Error konfiguracja GPIO : %s", strerror(errno));
    		close(fd);
    		return(-1);
	}

	data.values[0] = value;
	ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);

        if (ret == -1)
        {
                printf("Error ustawienie GPIO : %s", strerror(errno));
        }
	else
	{
	//	usleep(2000000);
	}

	close(rq.fd);
	return(0);
}

/*
fajna stronka
http://rosettacode.org/wiki/Keyboard_input/Keypress_check
*/
#include <termios.h>
#include <fcntl.h>

int set_mode(int want_key)
{
	static struct termios oldk, newk;
	if (!want_key)
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &oldk);
		return(-1);
	}

	tcgetattr(STDIN_FILENO, &oldk);
	newk = oldk;
	newk.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newk);

	return(0);
}

int get_key()
{
	int c = 0;
	struct timeval tv;
	fd_set fs;
	tv.tv_usec = tv.tv_sec = 0;

	FD_ZERO(&fs);
	FD_SET(STDIN_FILENO, &fs);

	select(STDIN_FILENO + 1, &fs, 0, 0, &tv);

	if (FD_ISSET(STDIN_FILENO, &fs))
	{
		c = getchar();
		set_mode(0);
	}
	return(c);
}

int main()
{
	/* INFO o chipie */
	//get_info();

	/* INFO o liniach */
	//get_line_info(info.lines);

	int pin = 12;
	char znak = 0;
	int dongle = 0;

	while(1)
	{
		set_mode(1);
		while(!(znak = get_key()))
		{
			usleep(50000);
		}

		if(znak != 'q')
		{
			if(dongle == 0)
			{
				dongle = 1;
				set_gpio(pin, 1);
			}
			else
			{
				dongle = 0;
				set_gpio(pin, 0);
			}
		}
		else
		{
			set_mode(0);
			return(0);
		}
	}
	return(0);
}
