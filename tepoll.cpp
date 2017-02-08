#include <pthread.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>


#define MAX_EVENT_NUM 10240
#define BUFFER_SIZE 5

static int SetNonBlocking(int fd)
{
	int old_flag = fcntl(fd, F_GETFL);
	int new_flag = old_flag | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_flag);
	return old_flag;
}


void addfd(int epollfd, int fd, bool enable_et)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if(enable_et)
	{
		event.events |= EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	SetNonBlocking(fd);
}

void lt(epoll_event* events, int number, int epollfd, int listenfd)
{
	char buff[BUFFER_SIZE];
	for(int i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;
		if(sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address,
					&client_addrlength);
			addfd(epollfd, connfd, false);
		}
		else if( events[i].events & EPOLLIN )
		{
			printf("event trigger once\n");
			memset(buff, '\0', BUFFER_SIZE);
			int ret = recv(sockfd, buff, BUFFER_SIZE-1, 0);
			if(ret <= 0)
			{
				close(sockfd);
				continue;
			}
			printf("get %d bytes of content: %s\n", ret, buff);
		}
		else
		{
			printf("something happened!\n");
		}
	}
}

void et(epoll_event* events, int number, int epollfd, int listenfd)
{
	char buff[BUFFER_SIZE];
	for(int i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;
		if(sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address,
					&client_addrlength);
			addfd(epollfd, connfd, true);
		}
		else if( events[i].events & EPOLLIN )
		{
			printf("event trigger once\n");
			while(1)
			{
				memset(buff, '\0', BUFFER_SIZE);
				int ret = recv(sockfd, buff, BUFFER_SIZE-1, 0);
				if(ret < 0)
				{
					if(errno == EAGAIN || errno == EWOULDBLOCK)
					{
						printf("read later\n");
						break;
					}
					close(sockfd);
					break;
				}
				else if(ret == 0)
				{
					close(sockfd);
				}
				else
				{
					printf("get %d bytes of content: %s\n", ret, buff);
				}
			}
		}
		else
		{
			printf("something happened!\n");
		}
	}
}

int main(int argc, char* argv[])
{
	if(argc <= 2)
	{
		printf("usage: %s ip_address port_number\n", basename(argv[0]));
		return 1;
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(listenfd, 5);
	assert(ret != -1);

	epoll_event events[MAX_EVENT_NUM];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd, true);

	while(1)
	{
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);
		if(ret < 0)
		{
			printf("epoll fail\n");
			break;
		}
		et(events, ret, epollfd, listenfd);
	}

	close(listenfd);
	return 0;
}

