#include <stdio.h>
#include <linux/gpio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
/* klik bez enteru http://rosettacode.org/wiki/Keyboard_input/Keypress_check */
#include <termios.h>

#define GPIO_CHIP "/dev/gpiochip0"

int set_gpio(int pin, uint8_t value)
{
        int fd = 0;
	int ret = 0;

        struct gpiohandle_request rq;
        rq.lineoffsets[0] = pin;
        rq.lines = 1;
        rq.flags = GPIOHANDLE_REQUEST_OUTPUT;

	struct gpiohandle_data data;

        fd = open(GPIO_CHIP, O_RDONLY);
        if (fd < 0)
        {
                printf("Error dostep GPIO %s: %s", GPIO_CHIP, strerror(errno));
                return(-1);;
        }

	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
	close(fd);

	if (ret == -1)
	{
    		printf("Error konfiguracja GPIO : %s", strerror(errno));
    		return(-1);
	}

	data.values[0] = value;
	ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);

        if (ret == -1)
        {
                printf("Error ustawienie GPIO : %s", strerror(errno));
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

int call_sys(char *command)
{
        int result;
        result = system(command);

	if(result == -1)
	{
		printf("Error system() : %s", strerror(errno));
		return(-1);
	}

	if(result == 0)
        {
                return(0);
        }
        else
        {
                return(1);
        }
}

int interface_down_sys()
{
        call_sys("sudo ifconfig wlan0 down");
        call_sys("sudo ifconfig eth0 down");
        return(0);
}

int interface_up_sys()
{
        call_sys("sudo ifconfig wlan0 up");
        call_sys("sudo ifconfig eth0 up");
        return(0);
}

int ping_test_sys()
{
        int result = call_sys("ping -c1 -w1 8.8.8.8 > /dev/null 2>&1");

        if(result == 0)
        {
                printf("internet ok\n");
                return(1);
        }
        else if(result == 1)
        {
                printf("internet brak\n");
                return(0);
        }

	return(-1);
}

int ping_wait_internet(int ile, unsigned int co_ile, int oczekiwany_status_ping)
{
	int i = 0;
	int wynik = oczekiwany_status_ping ? 0 : 1;
	while(i != ile &&
	(wynik = ping_test_sys()) != oczekiwany_status_ping)
	{
		usleep(co_ile);
		i++;
	}
	return(wynik);
}

const char *info = "q\t\t\t- konczy program, ustawia pin diody LED na 0\n"
		   "klawisz inny niz q\t- na przemian wlacza interfejsy sieciowe i\n"
		   "\t\t\tzapala diode lub wylacza interfejsy sieciowe\n"
		   "\t\t\ti gasi diode"
		   "\n\n";

int main()
{
	int pin = 12;
	char znak = 0;
	int stan_diody_led = 0;
	int ile_powtorzen = 50;
	int ile_czasu_us = 500000;

	printf(info);

	if(ping_wait_internet(500, 500000, 1) == 1)
	{
		stan_diody_led = 1;
	}
	set_gpio(pin, stan_diody_led);


	while(1)
	{
		set_mode(1);
		while(!(znak = get_key()))
		{
			usleep(50000);
		}

		if(znak != 'q')
		{
			if(stan_diody_led == 0 &&
			interface_up_sys() == 0 &&
			ping_wait_internet(ile_powtorzen, ile_czasu_us, 1) == 1)
			{
				stan_diody_led = 1;
			}
			else if(interface_down_sys() == 0 &&
			ping_wait_internet(ile_powtorzen, ile_czasu_us, 0) == 0)
                        {
				stan_diody_led = 0;
			}
		}
		else if(znak == 'q')
		{
			set_gpio(pin, 0);
			set_mode(0);
			return(0);
		}

		set_gpio(pin, stan_diody_led);
	}
	return(0);
}
