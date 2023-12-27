#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MSG_LEN 1500

char * decode (char * inputString)
{
	int len = strlen(inputString); 

    char *ans = (char *) malloc(2000 * sizeof(char)); 

    int numBits = 0;
    int val = 0;
    int resIndex = 0;
    int count=0;

    for (int i = 0; i < len; i += 4) 
    {
    	val=0;
    	numBits=24;
    	count=0;
    	for(int j=i;j<=i+3 && j<len ; j++)
    	{
    		
    		int temp_index;
			if(inputString[j]>='A' && inputString[j]<='Z')
				temp_index = inputString[j]-'A';
			else if(inputString[j]>='a' && inputString[j]<='z')
				temp_index = inputString[j]-'a'+26;
			else if(inputString[j]>='0' && inputString[j]<='9')
				temp_index = inputString[j]-'0'+52;
			else if(inputString[j]=='+')
				temp_index=62;
			else if(inputString[j]=='/')
				temp_index=63;
    		else if (inputString[j]=='=')
    		{
    			count++;
    			continue;
    		}
    		val = val << 6;
			val = val | temp_index;
    	}

    	if(count==2)
    	{
    		val = val >> 4;
    		numBits=8;
    	}
    	else if(count==1)
    	{
    		val = val >> 2;
    		numBits=16;
    	}
   

    	for(int j=8;j<=numBits;j+=8)
    	{
    		int temp=val >> (numBits-j);
    		int index=temp%256;
    		ans[resIndex++]=(char)index;
    	}
    	
    }
    ans[resIndex]='\0';
    return ans;
}

















char * encode (char * inputString)
{
    int len = strlen(inputString);
    char set64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 

    char *ans = (char *) malloc(2000 * sizeof(char)); 

    int numBits = 0, pad = (3-len%3)%3;
    int val = 0;
    int resIndex = 0;

    for (int i = 0; i < len; i += 3) 
    {
    	val=0;
    	numBits=0;
    	for(int j=i;j<=i+2 && j<len ; j++)
    	{
    		val = val << 8;
    		val = val | inputString[j];
    		numBits+=8;
    	}

    	if(numBits==8)
    	{
    		val = val << 4;
    		numBits=12;
    	}
    	else if(numBits==16)
    	{
    		val = val << 2;
    		numBits=18;
    	}

    	for(int j=6;j<=numBits;j+=6)
    	{
    		int temp=val >> (numBits-j);
    		int index=temp%64;
    		char ch=set64[index];
    		ans[resIndex++]=ch;
    	}
    	
    }
    for(int j=0;j<pad;j++)
	{
		ans[resIndex++]='=';
	}
    ans[resIndex]='\0';

    return ans;
}













void recvAndAck(int c_fd,struct sockaddr_in *client) //c_fd is client socket
{
	char *ip = inet_ntoa(client->sin_addr); //ip address of client
	int port = client->sin_port; // port of client
	printf("\nNEW CLEINT CONNECTION (%s : %d) ESTABLISHED\n",ip,port);

	char buffer[MSG_LEN]; //buffer to store sending value

	//wait for msg from client till close connection request is recieved
	while(1)
	{
		bzero(buffer,MSG_LEN); 
		int st = read(c_fd,buffer,MSG_LEN); //read msg from socket into the buffer
		
		//first char of msg is msg type. 3 refers to close connection
		if(buffer[0]=='3')
			break;

		if(buffer[0]=='1')
		{
			printf("\nMessage received from client %s : %d\n\tEncoded Message: %s\n",ip,port,buffer+1); //print encoded msg
			printf("\tDecoded Message: %s\n",decode(buffer+1)); //print decoded msg

			bzero(buffer,MSG_LEN);
			buffer[0]='2'; //write to buffer. 2 refers to acknowledgement
			strcpy(buffer+1,encode("ACK")); //append encoded value of "ACK" after msg type
			write(c_fd,buffer,strlen(buffer)); //send acknowledgement
		}
		else
			break;
	}

	close(c_fd); //close connection
	printf("\nCLEINT CONNECTION (%s : %d) CLOSED\n",ip,port);
	exit(0);
}


int main(int argc, char *argv[])
{
	struct sockaddr_in server,client;
	int serverSocket = socket(AF_INET,SOCK_STREAM,0); //AF_NET is for IPv4, SOCK_STREAM indicates that TCP socket is created
	
	//couldnt create socket
	if(serverSocket == -1)
	{ 
		printf("\nSOCKET CREATION FAILURE\n");
		exit(0);
	}
	server.sin_family = AF_INET;  //code for address family - IPv4
	server.sin_addr.s_addr = INADDR_ANY; //address of host, binds the socket to all available local interfaces
	server.sin_port = htons(atoi(argv[1]));  //arguement 1 is server port number entered by user
	memset(&server.sin_zero,8, 0); 
	socklen_t length = sizeof(struct sockaddr_in);
	
	//if port is already in use binding fails
	if(bind(serverSocket,(struct sockaddr*) &server, length) < 0)
	{ 
		printf("\nBINDING FAILED\n");
		exit(0);
	}

	//listen to the current socket
	if(listen(serverSocket,20) == -1)
	{
		printf("\nLISTEN FAILED\n");
		exit(0);
	}
	printf("SERVER WORKING\n");
	fflush(stdout);
	while(1)
	{
		int clientSocket = accept(serverSocket,(struct sockaddr *) &client,&length); //client tries to connect
		
		// client connection was not made
		if(clientSocket < 0)
		{ 
			printf("\nSERVER-CLIENT CONNECTION COULD NOT BE ESTABLISHED\n");
			exit(0);
		}
		int status = fork(); //fork to create a child process to handle this client, so that multiple clients can be handled concurrently
		switch(status)
		{
			// error while creating child process
	 		case -1:
				printf("\nCOULDN'T ESTABLISH CONNECTION\n");
				break;
			//child process
			case 0:
				close(serverSocket); //server socket is handled by parent process
				recvAndAck(clientSocket,&client); // this functions handles a client
				break;
			//parent process 
			default:
				close(clientSocket);  // client socket is now handled by child process	
		}
		
	}
	return 0;


}
