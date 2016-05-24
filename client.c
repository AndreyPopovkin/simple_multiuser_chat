#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#define MAX_BUFSIZE 65536

void p_error(char* msg){
	perror(msg);
	exit(0);
}

int server_is_dead = 0;

void* read_msg(void* s){
	int n;
	int s_fd = *(int*)s;
	char buffer[MAX_BUFSIZE];
	bzero(buffer, MAX_BUFSIZE);
	while(buffer[0] != '!' || buffer[1] != 'O' || buffer[2] != 'K' 
														|| buffer[3] != '!'){
		bzero(buffer, MAX_BUFSIZE);
		n = read(s_fd, buffer, MAX_BUFSIZE);
		if (n < 0)
			p_error("Error while reading from socket");
		buffer[n] = 0;
		if (n == 0){
			server_is_dead = 1;
			pthread_exit(0);
		}
		printf("others: %s\n", buffer);
		puts("!");
	}
	server_is_dead = 1;
}

int main(int argc, char** argv){
	int sock_fd, port_no, status;
	struct sockaddr_in serveraddr;
	char* hostname;
	struct hostent* server;
	pthread_t thread;

/* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    port_no = atoi(argv[2]);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0)
		p_error("Error while opening socket");

	server = gethostbyname(hostname);
	if (server == NULL){
		fprintf(stderr, "No such host\n");
		exit(0);
	}

	bzero((char*)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_port = htons(port_no);
	bcopy((char*)server->h_addr, (char*)&serveraddr.sin_addr.s_addr, 
															server->h_length);
	serveraddr.sin_family = AF_INET;
	
	if (connect(sock_fd, (const struct sockaddr*)&serveraddr, 
												sizeof(serveraddr)) < 0)
	p_error("Error while connecting");

	status = pthread_create(&thread, NULL, read_msg, &sock_fd);
/*	if (!status)
		p_error("Error while thread creating");*/

	int i, c, n;
	char buf[MAX_BUFSIZE];
	do{
		bzero(buf, MAX_BUFSIZE);
		printf("you:\n");
		for (i = 0; i < MAX_BUFSIZE - 1; ++i){
        	c = fgetc(stdin);
        	if (c == '\n')
            	break;
        	buf[i] = c;
    	}
		buf[i] = 0;
		if(server_is_dead){
			puts("sorry, but server is dead");
			return 0;
		}
		if(i == 0)
			continue;
		n = write(sock_fd, buf, i);
		if (n < 0)
			p_error("Error while writing to socket");
	}while(buf[0] != '~' || buf[1] != 'q' || buf[2] != 'u' || buf[3] != 'i' || 
							buf[4] != 't');
	puts(buf);
	return 0;
}
