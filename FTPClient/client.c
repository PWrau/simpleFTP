#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>

#include <arpa/inet.h>

#define SENTENCE_LENGTH 1024

int buildConnection()
{
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

struct sockaddr_in buildSocket()
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = 6789;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	return addr;
}

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in addr;
	char sentence[SENTENCE_LENGTH];
	int len;
	int p;

	sockfd = buildConnection();
	if (sockfd == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	printf("reach socket\r\n");

	addr = buildSocket();
	
	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	printf("reach inet\r\n");

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	printf("reach connect\r\n");
	while (1)
	{
		memset(sentence, 0, sizeof(sentence));
		fgets(sentence, SENTENCE_LENGTH, stdin);
		len = strlen(sentence);
		sentence[len] = '\n';
		sentence[len + 1] = '\0';

		p = 0;
		while (p < len) {
			int n = write(sockfd, sentence + p, len + 1 - p);
			if (n < 0) {
				printf("Error write(): %s(%d)\n", strerror(errno), errno);
				return 1;
			}
			else {
				p += n;
			}
		}

		p = 0;
		while (1) {
			int n = read(sockfd, sentence + p, SENTENCE_LENGTH - 1 - p);
			if (n < 0) {
				printf("Error read(): %s(%d)\n", strerror(errno), errno);
				return 1;
			}
			else if (n == 0) {
				break;
			}
			else {
				p += n;
				if (sentence[p - 2] == '\n') {
					break;
				}
			}
		}

		sentence[p - 1] = '\0';

		printf("FROM SERVER: %s", sentence);
	}

	

	close(sockfd);

	return 0;
}
