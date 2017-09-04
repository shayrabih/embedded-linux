/* 
Project - Rs232 and Ethernet Loop 
File name: PC_server.c

This file is part of a final project for Embedded Linux Course

The project requires 5 files:
PC_server.c (THIS ONE)
PC_client.c
evb2.c
PROJECT.h
EVB2.h

Description
-----------
The server side creates a Domain socket and waits for the remote client on the EVB.
The server part should run before the client.
 
To compile me for Linux, use gcc -ggdb PC_server.c -o server
To execute, type:  ./server

Note: Please make sure firewall is down
for Centos use - service iptabes stop or systemctl stop firewalld
*/

#include "PROJECT.h"

int main()
{
        int sock,addr_len;
        ssize_t bytes_read;
        String recv_data;
        struct sockaddr_in server_addr ,client_addr;
	int dontblock,rc;

	printf("\n*************************************************\n");
	printf("* Wellcome to the PC_server implementation !\t*\n");
	printf("* Note: Please make sure firewall is down\t*\n");
	printf("* For Centos use: 1. service iptabes stop, or\t*\n");
	printf("*\t\t2. systemctl stop firewalld\t*\n");
	printf("*************************************************\n");

	/* Place the socket into nonblocking mode */
	dontblock = 0;

	//open socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		die("socket");

	rc = ioctl(sock, FIONBIO, (char *) &dontblock);

	//Fill in server_addr structure
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(5002);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(server_addr.sin_zero),8);

	//Bind socket to address
        if (bind(sock,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
		die("Bind");
       
        addr_len = sizeof(struct sockaddr);
		
	printf("\nUDPServer Waiting for client on port 5002\n");
	printf("-----------------------------------------\n\n");
        fflush(stdout);//flushes the output buffer of the stdout stream.

	while (1)
	{
		memset(recv_data,0,STR_SIZE);
		bytes_read = recvfrom(sock,recv_data,STR_SIZE,0, (struct sockaddr *)&client_addr, &addr_len);
if (bytes_read == -1)
				die("recvfrom");
		if (bytes_read > 0)
		{
			recv_data[bytes_read] = '\0';
			printf("PC_server: bytes_read=%d from (%s , %d) said: ",bytes_read,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
			printf("%s\n\n", recv_data);
			fflush(stdout);
		}
		
		if (strcmp(recv_data,"Terminate program request has accepted!") == 0)
		{
			break;
		}
		sleep(1);
	}

	printf("Now closing the UDP socket...\n");
	close(sock);
	printf("\nUDPServer no longer waiting for client on port 5002!\n");

	return 0;
}
