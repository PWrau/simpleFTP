#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#include "Serve.h"
#include <arpa/inet.h>
#include "Global.h"

int checkStatus(int res, int failStatus)
{
	if (res == failStatus)
	{
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	return 0;
}

int buildConnection()
{
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

struct sockaddr_in buildSocket()
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = INIT_PORT;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	return addr;
}

int buildBindAndListenConnection(int listenfd, struct sockaddr_in serverAddr)
{

	if (checkStatus(bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)), STATUS_FAIL) == 1)
	{
		return 1;
	}

	printf("reach bind\r\n");

	if (checkStatus(listen(listenfd, 20), STATUS_FAIL) == 1)
	{
		return 1;
	}

	printf("reach listen\r\n");

	return 0;
}

int main(int argc, char **argv) {
	
	// fd: file decorator
	int listenfd, connfd;
	struct sockaddr_in serverAddr, clientAddr;
	char sentence[SENTENCE_LENGTH];
	char buffer[SENTENCE_LENGTH];
	socklen_t clientLength;
	int p;
	int len;

	listenfd = buildConnection();
	if (checkStatus(listenfd, STATUS_FAIL) == 1) {
		return 1;
	}

	printf("reach socket\r\n");

	// epoll

	
    // events可以是以下几个宏的集合：
    // (枚举类型)
    // EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
    // EPOLLOUT：表示对应的文件描述符可以写；
    // EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
    // EPOLLERR：表示对应的文件描述符发生错误；
    // EPOLLHUP：表示对应的文件描述符被挂断；
    // EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
    // EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里
	struct epoll_event ev, events[EVENT_SIZE];
	// epfd: 创建的句柄
    // create的大小大于等于wait中的最大事件数
	int epfd = epoll_create(FD_MAX_SIZE);
	
	ev.data.fd = listenfd;
	ev.events = EPOLLIN | EPOLLET;

	// EPOLL_CTL_ADD：注册新的fd到epfd中；
	// EPOLL_CTL_MOD：修改已经注册的fd的监听事件；
	// EPOLL_CTL_DEL：从epfd中删除一个fd；
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
	// .
	
	memset(&clientAddr, 0, sizeof(clientAddr));
	serverAddr = buildSocket();

	if (buildBindAndListenConnection(listenfd, serverAddr) != 0)
	{
		return 1;
	}

	// epoll
	int nfds;
	int sockfd;
	ssize_t n;
	int count = 0;
	while (1)
	{
		count++;
		nfds = epoll_wait(epfd, events, EVENT_SIZE, -1);
        // nfds == 0 表明超时
		// printf("%d: wait %d\n", count, nfds);
		for (int i = 0; i < nfds; i++)
		{
			// 如果新监测到一个socket用户连接到了绑定的socket端口
			// 建立新的连接
			if (events[i].data.fd == listenfd)
			{
				connfd = accept(listenfd, (struct sockaddr*) &clientAddr, &clientLength);
				if (connfd < 0)
				{
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					return 1;
				}

				char *str = inet_ntoa(clientAddr.sin_addr);
				printf("connect client %s\n", str);

				ev.data.fd = connfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
			}

			// 如果是已经连接的用户，并且收到数据，那么进行读入
			else if (events[i].events & EPOLLIN)
			{
				printf("enter read data\n");
				sockfd = events[i].data.fd;
				if (sockfd < 0)
				{
					continue;
				}
				n = read(sockfd, buffer, SENTENCE_LENGTH);
                // 如果连接错误
				if (n < 0)
				{
					if (errno == ECONNRESET)
					{
						close(sockfd);
						events[i].data.fd = -1;
					}
					else
					{
						printf("readline err\n");
					}
				}
                // 如果没有读到数据
				else if (n == 0)
				{
					close(sockfd);
					events[i].data.fd = -1;
				}
				// n > 0
				buffer[n] = '\0';
				printf("receive: %s\n", buffer);

				ev.data.fd = sockfd;
				ev.events = EPOLLOUT | EPOLLET;

				// 准备写
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                // 不可以关闭sockfd
			}
			// 如果有数据发送
			else if (events[i].events & EPOLLOUT)
			{
				//printf("enter send data\n");
				sockfd = events[i].data.fd;
				write(sockfd, buffer, n);

				ev.data.fd = sockfd;
				ev.events = EPOLLIN | EPOLLET;
				// 写完后准备读
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                // 不可以关闭sockfd
			}
		}
	}
	// .
	close(connfd);
	close(epfd);
	
	return 0;
}

