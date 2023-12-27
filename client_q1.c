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
#define MSG_LEN





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











void getInput(char buffer[])
{
	char inp;
	printf("Write message to send\n");
	bzero(buffer, MSG_LEN); 
	int i = 0;
	
	//maximum input size is 1000
	while(1)
	{
		scanf("%c", &inp); //read message character by character    
		if(inp == '\n') 
			break;
		if (i == 1000)
		{
			printf("Only first 1000 characters of message are being sent\n");
			break;
		}
		buffer[i++] = inp; 
	}
}

void closeConnection(char buffer[], int clientSocket)
{
	buffer[0] =  '3'; 	 //set msg type
	strcpy(buffer+1, encode("connection_close")); // append encoded value of "connection_close" after msg type
	printf("CONNCECTION CLOSED\n"); 
	write(clientSocket, buffer, strlen(buffer)); // write in socket
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server; 
	char inp; //to read user input character by character 
	int clientSocket = socket(AF_INET,SOCK_STREAM,0); //AF_NET is for IPv4, SOCK_STREAM indicates that TCP socket is created
	char buffer[MSG_LEN]; //buffer to store recieving and modified sending value
	
	//socket couldn't be created
	if(clientSocket == -1)
	{
		printf("SOCKET CREATION FAILURE\n");
		return 0;
	} 

	server.sin_family = AF_INET; //address family - ipv4

	//server ip address, inputted by user
	if(inet_aton(argv[1],&server.sin_addr)==0)
	{ 
		printf("SERVER IP ADDRESS ERROR\n");
		return 0;
	} 

	int serverPort = atoi(argv[2]); // server port, inputted by user
	server.sin_port = htons(serverPort); 
	socklen_t length = sizeof(struct sockaddr_in); 

	// connection establishment failure
	if(connect(clientSocket,(struct sockaddr *) &server, length) == -1)
	{ 
		printf("COULDN'T CONNECT TO THE SERVER\n");
		exit(0);
	} 

	while(1)
	{
		
		printf("Send message?\nPress 'y' for YES or any other key for NO and press enter: \n");
		scanf("%c", &inp);

		//n impies send close connection request to server
		if(inp != 'y')
		{
			closeConnection(buffer,clientSocket);
			break; 
		}

		getchar();
		getInput(buffer);

		strcpy(buffer+1, encode(buffer)); // append encoded value of msg after msg type
		buffer[0] =  '1'; //set msg type as 1
		write(clientSocket, buffer, strlen(buffer)); //write to socket
		bzero(buffer, 1500);
		int received = read(clientSocket, buffer, 50); //recieve acknowledgement from server
		strcpy(buffer+1, decode(buffer+1)); //decode msg

		//msg type 2 indicates acknowledgement
		if(buffer[0]!='2')
		{
			printf("Acknowledgement could not received.\nResend the message.\n"); 
			continue; 
		}

		
		printf("Message received from server having IP %s and port %d\n",argv[1], serverPort); 
		printf("%s\n", buffer+1);
	}

	close(clientSocket); // close the socket

return 0;
}
