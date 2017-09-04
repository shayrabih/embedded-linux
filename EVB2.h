/* 
Project - Rs232 and Ethernet Loop 
File name: EVB.h


This file is part of a final project for Embedded Linux Course

The project requires 5 files:
PC_server.c
PC_client.c
evb2.c
PROJECT.h
EVB2.h (THIS ONE)
*/

#ifndef EVB2_H
#define EVB2_H

////////////////////////////* Prototypes */////////////////////

//Sockets
int init_server_udp_socket(struct sockaddr_in *server_addr);
int init_client_udp_socket(char *remote_server_ip_addr);
int recv_data_udp_socket(void* fd_sock);
int send_data_udp_socket(void* fd_sock);

//Uart
int init_con_uart(int uart_num);
int send_data_to_uart(void* fd);
int read_data_from_uart(void* fd);

////////////////////////////* Globals *///////////////////////

String sock_recv_buf;
String uart_recv_buf;
struct sockaddr_in remote_server_addr;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_cond_t cond_var1;
pthread_cond_t cond_var2;

#endif /*EVB2_H*/

