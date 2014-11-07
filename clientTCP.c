////Manuel Perez ECE 155A HW7 TCP Client


// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket-related libraries
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>

#define MAX_LINE_SIZE 50

void print_usage(){
  fprintf(stderr, "Usage: ./client [hostname] [port]\n");
  fprintf(stderr, "Example: ./client 192.168.0.2 4845\n\n");
}
 
void check_args(int argc, char *argv[]){
  if( argc != 3 ) {
    print_usage();
	exit(1);
  }
}

int main(int argc, char *argv[])
{

  struct addrinfo hints;
  struct addrinfo *serverInfo;
  int rv, i, ex, R=1, S=1;
  int socketFD;
  //in loop
  int recvFD;
  char ack[5];
  char exitS[]="exit";
  int num_bytes_received;
  
  // Check arguments and exit if they are not well constructed
  check_args(argc, argv);
  
  // Fill the 'hints' structure
  memset( &hints, 0, sizeof(hints) );
  hints.ai_family   = AF_INET;     // Use IPv4 address
	hints.ai_socktype = SOCK_STREAM; // TCP sockets
  
	// Get the address information of the server.
	// argv[1] is the address, argv[2] is the port
	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &serverInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
  
	// Create a socket to communicate with the server
	if ((socketFD = socket( serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol )) == -1)
         perror("clientTCP: socket");
		 
	if (serverInfo == NULL) {
        fprintf(stderr, "client: failed to bind socket\n");
        return 2;
    }
    // Connect to the server (only for TCP)
	if((connect(socketFD, serverInfo->ai_addr, serverInfo->ai_addrlen)) == -1)
	{
		perror("clientTCP: connect");
		exit(1);
	}
	
	printf("Waiting for server connection...\n");
	
	//ack recieved
	if ((num_bytes_received = recv(socketFD, ack, MAX_LINE_SIZE-1, 0)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	freeaddrinfo(serverInfo);
	if(ack[0]=='a')
		printf("Received Ack: connected server R and server S\n");
	else if(ack[0]=='o')
		{
			printf("Received Ack: connected to server R only\n");
			S=0;
		}
	else if(ack[0]=='t')
		{
			printf("Received Ack: connected to server S only\n");
			R=0;
		}
	else
	{
		printf("Error: gateway not connected to any server\n");
		return 0;
	}
	
	while(1){
	// Ask for data to send
	char line[MAX_LINE_SIZE];
	char returnedString[MAX_LINE_SIZE];
	int len = 0;
	int n;
  
		while(1)
		{
			for(i=0; i<len ; i++) //Clean array
				line[i]='\0';
			//good=1;
			printf("Please input the string to send: ");
			scanf(" %50[^\n]s", line);
			len = strlen(line);
			if(len<3)
			{
				printf("Wrong input: please follow format: 'string k S' or 'string R'\n");
				continue;
			}
			ex=1;
			if(len==4)
			{
				for(i=0; i<4 ;i++)
				{
					if(line[i] != exitS[i])
					{
						ex=0;
						break;
					}
				}
			}
			if(ex==1 && len==4)
			{
				break;
			}
			else if(line[len-1]=='R' && line[len-2]==' ' && R==0)
			{
				printf("Server R is unavailable\n");
				continue;
			}
			else if(line[len-1]=='S' && line[len-2]==' ' && S==0)
			{
				printf("Server S is unavailable\n");
				continue;
			}
			else if((line[len-1]!='R' && line[len-1]!='S') || line[len-2]!=' ')
			{
				printf("Wrong input: no S or R specified\n");
				continue;
			}
			if(line[len-1]=='S')
			{
				if(len<5)
				{
					printf("Wrong input: please follow format: 'string k S' or 'string R'\n");
					continue;
				}
				if(line[len-4]==' ')
				{
					n=4;
					if(line[len-3]<'1'||line[len-3]>'9')
					{
						printf("Wrong input: please follow format: 'string k S' or 'string R'\n");
						continue;
					}
				}
				else if((line[len-3]<'0'||line[len-3]>'9')||(line[len-4]<'1'||line[len-4]>'2')||(line[len-3]>'5'&&line[len-4]=='2')||line[len-5]!=' '||len-5==0)
				{
					printf("Wrong input: please follow format: 'string k S' or 'string R'\n");
					continue;
				}
				else
					n=5;
			}
			else
				n=2;
			int good=1;
			for(i=0; i<len-n;i++)
			{
				if((line[i]<'a'||line[i]>'z')&&(line[i]<'A'||line[i]>'Z')&&line[i]!=' ')
				{
					printf("Wrong input: only letters and spaces allowed\n");
					good=0;
					break;
				}
			}
			if(good==0)
				continue;
			break;
		}
  
  
	// Send the data:
	printf("Sending string '%s' to '%s' using port '%s'... ", line, argv[1], argv[2]);
  
	if ((send(socketFD, line, len, 0)) == -1) {
        perror("clientTCP: send");
        exit(1);
    }
	
	if(len==4 && ex==1)
	{
		printf("\nSession Closed\n");
		break;
	}
	
	else // TCP style
		printf("Done!\n\n");
		
	//Clean array
	for(i=0; line[i]!='\0' ; i++)
		line[i]='\0';
		
	//data recieved
	if ((num_bytes_received = recv(socketFD, returnedString, MAX_LINE_SIZE-1, 0)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	
	returnedString[num_bytes_received]='\0';
	
	printf("   Encrypted string received: '%s'\n\n", returnedString);
	fflush(stdout);
    
	//Clean array
	for(i=0; returnedString[i]!='\0' ; i++)
		returnedString[i]='\0';
	}
  // Free the address info struct and close the socket
  close(socketFD);
  
  return 0;
}