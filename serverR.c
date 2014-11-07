//Manuel Perez ECE 155A HW7 TCP Server R

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket-related libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>

#define ALLOC(num,type)    (type *)calloc(num,sizeof(*type))
#define MAX_LINE_SIZE 50
#define LISTEN_QUEUE 5
#define NUM_THREADS  2

char* port;
int myQueue[NUM_THREADS];
pthread_t threads[NUM_THREADS];
pthread_mutex_t mutexsum;

void print_usage(){
  fprintf(stderr, "Usage: ./server [port]\n");
  fprintf(stderr, "Example: ./server 4845\n\n");
}
 
void check_args(int argc, char *argv[]){
  if( argc != 2 ) {
    print_usage();
	exit(1);
  }
}

void *PrintHello(void *threadid)
{
	char exitS[]="exit";
	int key;
	long tid;
	int recvFD;

	tid = (long)threadid;
	recvFD=tid;
	printf("\nThread #%i initialized",recvFD-3);
	
	    // Receive data
    char originalString[MAX_LINE_SIZE];
	char encryptedString[MAX_LINE_SIZE];
	int num_bytes_received;
	int lineLen;
	int i, ex;

	while(1){
	
		printf("\nListening in port %s...", port);
		fflush(stdout);
		
		if ((num_bytes_received = recv(recvFD, originalString, MAX_LINE_SIZE-1, 0)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		printf("\n");
	

		//EXIT	
		ex=1;
		if(num_bytes_received==4)
		{
			for(i=0; i<4 ;i++)
			{
				if(originalString[i] != exitS[i])
				{
					ex=0;
					break;
				}
			}
		}
		if(num_bytes_received==4 && ex==1)
			break;
		
		for(i=0; i<num_bytes_received; i++)
		{
			encryptedString[i]=originalString[num_bytes_received-1-i];
		}
		encryptedString[num_bytes_received]='\0';
		
		// Print received data
		printf("Your reversed string is '%s'\n",encryptedString);

		lineLen = strlen(encryptedString);
		if ((send(recvFD, encryptedString, lineLen, 0)) == -1) {
			perror("serverR: send");
			exit(1);
		}

		//Clean array
		for(i=0; originalString[i]!='\0' ; i++)
		{
			originalString[i]='\0';
		}
	
		for(i=0; encryptedString[i]!='\0' ; i++)
		{
			encryptedString[i]='\0';
		}
	}
	
	printf("Closing Thread \n");
	pthread_mutex_lock (&mutexsum);
	//Clean Queue Array Up
	for(i=0; i<NUM_THREADS; i++)
	{
		if(recvFD==myQueue[i])
		{
			myQueue[i]=-1;
		}
	}
	pthread_mutex_unlock (&mutexsum);
	// Close the socket associated to this client
    close(recvFD);
	
	printf("\nListening in port %s...", port);
	fflush(stdout);
	
	pthread_exit(NULL);
	//pthread_exit((void*) 0);
  
}

/////////////////////
// MAIN /////////////
int main(int argc, char *argv[])
{
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int rv, q;
	int socketFD;

  
	//Used for threading
	
	int rc;
	long t;
  
	// Check arguments and exit if they are not well constructed
	check_args(argc, argv);
	
	port=argv[1];
	
	pthread_mutex_init(&mutexsum, NULL);
  
	// Fill the 'hints' structure
	memset( &hints, 0, sizeof(hints) );
	hints.ai_flags    = AI_PASSIVE;  // Use the local IP address
	hints.ai_family   = AF_INET;     // Use IPv4 address
	hints.ai_socktype = SOCK_STREAM; // TCP sockets
  
	// Get the address information of the server ('NULL' uses the local address )
	if ((rv = getaddrinfo( NULL , argv[1], &hints, &serverInfo )) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
  
	// Make a socket (identified with a socket file descriptor)
	if ((socketFD = socket( serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol )) == -1)
		perror("serverR: socket");

	// Bind to the specified port (in getaddrinfo())
	if (bind( socketFD, serverInfo->ai_addr, serverInfo->ai_addrlen ) == -1) {
			close(socketFD);
			perror("serverR: bind");
		}
  
	// This is only used to be able to reuse the same port
	int yes = 1;
	setsockopt( socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) );
  
	// Start to listen (only for TCP)
  
	if((listen(socketFD, LISTEN_QUEUE))== -1)
	{
		perror("serverR: listen");
		exit(1);
	}
  
	printf("Listening in port %s...", argv[1]);
	fflush(stdout);
  
	// Free the address info struct
	freeaddrinfo(serverInfo);
  
	// Accept a connections (only for TCP) and create a socket for each one
  
	struct sockaddr_storage clientAddr;
	socklen_t clientAddr_size = sizeof(clientAddr);
	int recvFD;
	char exitS[]="exit";
	
	//initialize Queue
	for(q=0; q<NUM_THREADS; q++)
	{
		myQueue[q]=-1;
	}
  
	while (1)
	{
		//check to see if any threads available
		for(q=0; q<NUM_THREADS; q++)
		{
			if(myQueue[q]==-1)
			{
				break;
			}
		}

		if( q==NUM_THREADS){
			sleep(1);
			continue;
		}
		if((recvFD = accept(socketFD, (struct sockaddr *)&clientAddr, &clientAddr_size))== -1){
			perror("accept");
			exit(1);
		}
		
		myQueue[q]=recvFD;
		t=(long)recvFD;
		
		//Creating new thread
		rc = pthread_create(&threads[q], NULL, PrintHello, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
  
	// Close the socket that was used to listen
	close(socketFD);
  
	/* Last thing that main() should do */
	pthread_exit(NULL);
  
	return EXIT_SUCCESS;
}