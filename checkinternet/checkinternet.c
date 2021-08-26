#include <stdlib.h>
#include <stdio.h>

int ping_test_sys()
{
	const char *target = "8.8.8.8";
	int result;

	result = system("ping -c1 -w1 8.8.8.8 > /dev/null 2>&1");

	if(result == 0)
	{
		printf("internet ok\n");
		return(1);
	}
        else
	{
		printf("internet brak\n");
		return(0);
	}
}

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int ping_test_socket(int port, char *address)
{
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr = {AF_INET, htons(53), inet_addr("8.8.8.8")};

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

        if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) != 0)
	{
                printf("internet brak\n");
		return(0);
	}
        else
	{
                printf("internet ok\n");
		return(1);
	}

        close(sockfd);

        return 0;
}


int call_sys(char *command)
{
        int result;
        result = system(command);

        if(result == 0)
        {
		//printf("sys call return 0\n");
                return(0);
        }
        else
        {
		//printf("sys call return 1\n");
                return(1);
        }
}

int interface_down_sys()
{
        call_sys("sudo ifconfig wlan0 down");
	call_sys("sudo ifconfig eth0 down");
//	usleep(500000);
	return(0);
}

int interface_up_sys()
{
        call_sys("sudo ifconfig wlan0 up");
        call_sys("sudo ifconfig eth0 up");
//	usleep(500000);
	return(0);
}


int main()
{
//	ping_test_sys();
	ping_test_socket(53, "8.8.8.8");

	interface_down_sys();
        ping_test_socket(53, "8.8.8.8");
	usleep(5000000);
//	ping_test_sys();
	interface_up_sys();
        ping_test_socket(53, "8.8.8.8");
//	ping_test_sys();
	return(0);
}
