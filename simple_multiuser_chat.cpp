#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <vector>
#include <pthread.h>
#include <string>

using std::vector;
using std::cout;
using std::cin;
using std::string;

const int MAX_BUFSIZE = 1000;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
vector<int> connections;

void print_error(string msg) {
    perror(msg.c_str());
    exit(0);
}

void* service(void* arg){

	int conn = *((int*)arg);
	while(1) cout << conn << " ";
	int n;
	char buf[MAX_BUFSIZE];

	pthread_mutex_lock(&lock);
	connections.push_back(conn);
	pthread_mutex_unlock(&lock);

    while(1){
    	n = read(conn, buf, MAX_BUFSIZE);
	    if (n < 0){
	    	perror("ERROR reading from socket");
	    	pthread_exit(0);
	    }
	    buf[n] = 0;
	    if((string)buf == "quit\n") {
	    	//buf = "!OK!";
	    	buf[0] = '!';
	    	buf[1] = 'O';
	    	buf[2] = 'K';
	    	buf[3] = '!';
	    	buf[4] = 0;
	    	n = write(conn, buf, strlen(buf));
	    	pthread_exit(0);
	    }
		pthread_mutex_lock(&lock);
		for(int i = 0; i < connections.size(); ++i)
			if(connections[i] != conn){
		    	n = write(connections[i], buf, strlen(buf));
		    	if (n < 0){
					//pthread_mutex_unlock(&lock);
		        	perror("ERROR writing to socket");
		        	cout << connections[i] << "\n";
		        }
			}
		pthread_mutex_unlock(&lock);
	}
	pthread_exit(0);
}

int main(int argc, char **argv)
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 10); 

    puts("!");

    //vector<pthread_t> threads;
    pthread_t thread;

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        //threads.push_back(pthread_t());
        pthread_create(&thread, NULL, service, &connfd);
        cout << connfd << " is connected\n";
     }
}