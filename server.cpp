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
//vector<int> connections;

void print_error(string msg) {
    perror(msg.c_str());
    exit(0);
}

map<int, pthread_t> threads;
//vector<int> terminated_clients;

int exit_from_servise(int id){

	cout << "terminating " << id << "\n";

	pthread_mutex_lock(&lock_map);
	threads.erase(id);
	close(id);
	pthread_mutex_unlock(&lock_map);
	//puts("?");
	pthread_exit(0);
}

void* service(void* arg){
	cout << "!\n";
	int conn = *((int*)arg);
	int* intarg = (int*)arg;
	delete intarg;
	int n;
	char buf[MAX_BUFSIZE];
	//cout << conn << "\n";
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
	    if(strncmp(buf, "~quit", 5) == 0) {
	    	buf[0] = '!';
	    	buf[1] = 'O';
	    	buf[2] = 'K';
	    	buf[3] = '!';
	    	buf[4] = 0;
	    	n = write(conn, buf, strlen(buf));
	    	//puts("!");
	    	exit_from_servise(conn);
	    }
	    //puts("~");
		pthread_mutex_lock(&lock_map);
		for(auto i = threads.begin(); i != threads.end(); ++i)
			if(i->first != conn){
		    	n = write(i->first, buf, strlen(buf));
				//puts(buf);
		    	if (n < 0){
					//pthread_mutex_unlock(&lock);
		        	perror("ERROR writing to socket");
		        	cout << i->first << "\n";
		        }else 
					cout << "send to " << i->first << "\n";
			}
		pthread_mutex_unlock(&lock_map);
	}
	exit_from_servise(conn);
}

int main(int argc, char **argv)
{
	if(argc != 2){
		cout << "using: ./server port\n";
		return 0;
	}

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 10); 

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    //cout << "please connecting to 5000 port\n";

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, 0); 
        //threads.push_back(pthread_t());
        cout << connfd << "\n";
        pthread_mutex_lock(&lock_map);
        //cout << "?\n";
        int* arg = new int(connfd);
		int err = pthread_create(&(threads[connfd] = pthread_t()), &attr, service, arg);
		if(err){
			perror("pthread_create\n");
			threads.erase(connfd);
			delete arg;
		}
		//cout << "!\n";
		pthread_mutex_unlock(&lock_map);
        cout << connfd << " is connected\n";
    }
    close(listenfd);
}
