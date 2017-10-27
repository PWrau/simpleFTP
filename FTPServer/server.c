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

	
    // events���������¼�����ļ��ϣ�
    // (ö������)
    // EPOLLIN ����ʾ��Ӧ���ļ����������Զ��������Զ�SOCKET�����رգ���
    // EPOLLOUT����ʾ��Ӧ���ļ�����������д��
    // EPOLLPRI����ʾ��Ӧ���ļ��������н��������ݿɶ�������Ӧ�ñ�ʾ�д������ݵ�������
    // EPOLLERR����ʾ��Ӧ���ļ���������������
    // EPOLLHUP����ʾ��Ӧ���ļ����������Ҷϣ�
    // EPOLLET�� ��EPOLL��Ϊ��Ե����(Edge Triggered)ģʽ�����������ˮƽ����(Level Triggered)��˵�ġ�
    // EPOLLONESHOT��ֻ����һ���¼���������������¼�֮���������Ҫ�����������socket�Ļ�����Ҫ�ٴΰ����socket���뵽EPOLL������
	struct epoll_event ev, events[EVENT_SIZE];
	// epfd: �����ľ��
    // create�Ĵ�С���ڵ���wait�е�����¼���
	int epfd = epoll_create(FD_MAX_SIZE);
	
	ev.data.fd = listenfd;
	ev.events = EPOLLIN | EPOLLET;

	// EPOLL_CTL_ADD��ע���µ�fd��epfd�У�
	// EPOLL_CTL_MOD���޸��Ѿ�ע���fd�ļ����¼���
	// EPOLL_CTL_DEL����epfd��ɾ��һ��fd��
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
        // nfds == 0 ������ʱ
		// printf("%d: wait %d\n", count, nfds);
		for (int i = 0; i < nfds; i++)
		{
			// ����¼�⵽һ��socket�û����ӵ��˰󶨵�socket�˿�
			// �����µ�����
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

			// ������Ѿ����ӵ��û��������յ����ݣ���ô���ж���
			else if (events[i].events & EPOLLIN)
			{
				printf("enter read data\n");
				sockfd = events[i].data.fd;
				if (sockfd < 0)
				{
					continue;
				}
				n = read(sockfd, buffer, SENTENCE_LENGTH);
                // ������Ӵ���
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
                // ���û�ж�������
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

				// ׼��д
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                // �����Թر�sockfd
			}
			// ��������ݷ���
			else if (events[i].events & EPOLLOUT)
			{
				//printf("enter send data\n");
				sockfd = events[i].data.fd;
				write(sockfd, buffer, n);

				ev.data.fd = sockfd;
				ev.events = EPOLLIN | EPOLLET;
				// д���׼����
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                // �����Թر�sockfd
			}
		}
	}
	// .
	close(connfd);
	close(epfd);
	
	return 0;
}

