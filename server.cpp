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
#include <map>

using std::vector;
using std::cout;
using std::cin;
using std::string;
using std::map;

const int MAX_BUFSIZE = 1000;

pthread_mutex_t lock_map = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t lock_ret = PTHREAD_MUTEX_INITIALIZER;
vector<int> connections;

void print_error(string msg) {
    perror(msg.c_str());
    exit(0);
}

map<int, pthread_t> threads;
//vector<int> terminated_clients;

int exit_from_servise(int id, int ret = 0){
	close(id);

	//pthread_mutex_lock(&lock_ret);
	//terminated_clients.push_back(id);
	//pthread_mutex_unlock(&lock_ret);

	pthread_mutex_lock(&lock_map);
	threads.erase(id);
	pthread_mutex_unlock(&lock_map);

	pthread_exit((void*)ret);
}

void* service(void* arg){
	int conn = *((int*)arg);
	int* intarg = (int*)arg;
	delete intarg;
	int n;
	char buf[MAX_BUFSIZE];

    while(1){
    	n = read(conn, buf, MAX_BUFSIZE);
		if (n < 0){
	    	perror("ERROR reading from socket");
	    	exit_from_servise(conn);
	    }
	    buf[n] = 0;
		if (!n){
			printf("Error: client %d died :c\n", conn);
	    	exit_from_servise(conn);
		}
		puts(buf);
		//cout << n << "\n";
	    if((string)buf == "quit") {
	    	buf[0] = '!';
	    	buf[1] = 'O';
	    	buf[2] = 'K';
	    	buf[3] = '!';
	    	buf[4] = 0;
	    	n = write(conn, buf, strlen(buf));
	    	exit_from_servise(conn);
	    }
		pthread_mutex_lock(&lock_map);
		for(auto i = threads.begin(); i != threads.end(); ++i)
			if(i->first != conn){
		    	n = write(i->first, buf, strlen(buf));
				//puts(buf);
				cout << "send to " << i->first << "\n";
		    	if (n < 0){
					//pthread_mutex_unlock(&lock);
		        	perror("ERROR writing to socket");
		        	cout << i->first << "\n";
		        }
			}
		pthread_mutex_unlock(&lock_map);
	}
	exit_from_servise(conn);
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

    //puts("!");

    //vector<pthread_t> threads;
    
	//pthread_mutex_lock(&lock2);

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        //threads.push_back(pthread_t());
        cout << connfd << "\n";
        pthread_mutex_lock(&lock_map);
		pthread_create(&(threads[connfd] = pthread_t()), NULL, service, new int(connfd));
		pthread_mutex_unlock(&lock_map);
        cout << connfd << " is connected\n";
    }
    close(listenfd);
}
