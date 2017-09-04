/* 
Project - Rs232 and Ethernet Loop 
File name: PC_client.c

This file is part of a final project for Embedded Linux Course

The project requires 5 files:
PC_server.c 
PC_client.c (THIS ONE)
evb2.c
PROJECT.h
EVB2.h

Description
-----------
The client side creates a Domain socket and starts writing to the remote server on the EVB.
The server part should run before the client.
 
To compile me for Linux, use gcc -ggdb PC_client.c -o client
To execute, type:  ./client
*/ 

#include "PROJECT.h"
#include <netdb.h>

int main()
{
	int sock;
	String str, str2;
	struct sockaddr_in server_addr;
	struct hostent *host;
	char *remote_server_ip_addr, *send_data;
	size_t send_len,to_send;
	ssize_t bytes_sent;

	printf("\n*************************************************\n");
	printf("* Wellcome to the PC_client implementation !\t*\n");
	printf("* Note: Please make sure firewall is down\t*\n");
	printf("* For Centos use: 1. service iptabes stop, or\t*\n");
	printf("*\t\t2. systemctl stop firewalld\t*\n");
	printf("*************************************************\n\n");

	//Get remote server address
	printf("remote_server_ip_addr (EVB): ");
	scanf("%s",str);
	remote_server_ip_addr = (char*)malloc((strlen(str)+1));
	
	if(remote_server_ip_addr == NULL)
	{
		printf("Out of memory, exiting\n");
		exit(1);
	}

	strcpy(remote_server_ip_addr,str);

	//Get host info
	host= (struct hostent *) gethostbyname((char *)remote_server_ip_addr);

	if (host == NULL)
		die("gethostbyname");

	//Open socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		die("socket");

	//Fill in server_addr structure
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8);//sets the first 8 bytes of server_addr.sin_zero to zero

	printf("\nNow you can start transmit messages\n");
	printf("-----------------------------------\n");

	while (1)
	{
		printf("\nType Something (q or Q to quit): ");
		scanf(" %[^\n]%*c",str2);
		send_data = (char*)malloc((strlen(str2)+1));
	
		if(send_data == NULL)
		{
			printf("Out of memory, exiting\n");
			exit(-1);
		}

		strcpy(send_data,str2);
		memset(str2,0,STR_SIZE);//Clear str2 for the next iteration
		send_len = strlen(send_data);
		to_send=send_len;

		if ((strcmp(send_data , "q") == 0) || (strcmp(send_data , "Q") == 0))
		{
			strcpy(send_data,"Terminate program request has accepted!");
			send_len = strlen(send_data);
			to_send=send_len;
			while(to_send)
			{
				bytes_sent=sendto(sock, send_data, send_len, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));//For UDP
         			printf("send_data_udp_socket(): bytes_sent=%d to (%s , %d) said : %s\n\n",bytes_sent,inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port),send_data);
				send_data += bytes_sent;
				to_send -= bytes_sent;
			}
			send_data = NULL;
			break;
		}
		else //Start sending 
		{
			while(to_send)
			{
				bytes_sent=sendto(sock, send_data, send_len, 0,(struct sockaddr *)&server_addr, sizeof(struct sockaddr));//For UDP
          			printf("\nsend_data_udp_socket(): send_len=%d bytes_sent=%d to (%s , %d) said : %s\n\n",send_len,bytes_sent,inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port),send_data);
				send_data += bytes_sent;
				to_send -= bytes_sent;
			}
		}
	send_data = NULL;
	}

	//Free dynamic memory allocated
	free_mem(send_data);
	free_mem(remote_server_ip_addr);

	printf("Now closing the UDP socket...\n");
	if (close(sock) == -1)
		die("Close socket");

	printf("Program has been terminated successfuly\n");

	return 0;
}
