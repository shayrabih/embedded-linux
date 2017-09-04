/* 
Project - Rs232 and Ethernet Loop 
File name: evb2.c

This file is part of a final project for Embedded Linux Course

The project requires 5 files:
PC_server.c
PC_client.c
evb2.c (THIS ONE)
PROJECT.h
EVB2.h

Description
-----------

  	
To compile me for Linux, use arm-linux-gcc -ggdb -lpthread evb2.c -o evb2
To execute, type:  ./evb2

Note: Please make sure firewall is down
For Centos use - service iptabes stop or systemctl stop firewalld
*/ 

/////////////////////////////////* Includes */////////////////////////

#include "PROJECT.h"
#include <sys/wait.h>   /* Wait for Process Termination */
#include <stdint.h>	/* Fixed-width integer types */
#include <termios.h>    /* Serial port */
#include <fcntl.h>	/* Operations on file descriptors */
#include <netdb.h>	/* Operations on Network database */
#include "EVB2.h"

//////////////////////////////////* main() func */////////////////////

int main()
{
        intptr_t fd_server_sock, fd_client_sock, fd_serial_1, fd_serial_2;
        int s, sock_bytes_read, uart_bytes_read, uart_bytes_write;
	pthread_t th1, th2, th3, th4;
        char *remote_server_ip_addr = NULL;
        struct sockaddr_in local_server_addr;

	printf("\n************************************************\n");
	printf("* Wellcome to the EVB implementation !\t\t*\n");
	printf("* Note: Please make sure firewall is down\t*\n");
	printf("* For Centos use: 1. service iptabes stop, or\t*\n");
	printf("*\t\t2. systemctl stop firewalld\t*\n");
	printf("************************************************\n");

	/* Initialize the mutexs. */
	pthread_mutex_init (&mutex1, NULL);
	pthread_mutex_init (&mutex2, NULL);

	/* Initialize the condition variables. */
	pthread_cond_init(&cond_var1, NULL);
	pthread_cond_init(&cond_var2, NULL);
	
	//open udp server socket	    
	fd_server_sock = init_server_udp_socket(&local_server_addr);
//	printf("fd_server_sock=%d\n", fd_server_sock);

	//open udp client socket
	fd_client_sock = init_client_udp_socket(remote_server_ip_addr);
//	printf("fd_client_sock=%d\n", fd_client_sock);

	//open and init serial 1
	fd_serial_1 = init_con_uart(1);
//	printf("fd_serial_1=%d\n", fd_serial_1);

	//open and init serial 2
	fd_serial_2 = init_con_uart(2);
//	printf("fd_serial_2=%d\n", fd_serial_2);

	//Threads create
	s = pthread_create(&th1, NULL, recv_data_udp_socket,(void *)fd_server_sock);
	if (s != 0) die("th1 pthread_create");
	s = pthread_create(&th2, NULL, send_data_to_uart,(void *) fd_serial_1);
	if (s != 0) die("th2 pthread_create");
	s = pthread_create(&th3, NULL, read_data_from_uart,(void *) fd_serial_2);
	if (s != 0) die("th3 pthread_create");
	s = pthread_create(&th4, NULL, send_data_udp_socket,(void *) fd_client_sock);
	if (s != 0) die("th4 pthread_create");

 	//Threads join
	s = pthread_join(th1, NULL);
	if (s != 0) die("th1 pthread_join");
	s = pthread_join(th2, NULL);
	if (s != 0) die("th2 pthread_join");
	s = pthread_join(th3, NULL);
	if (s != 0) die("th3 pthread_join");
	s = pthread_join(th4, NULL);
	if (s != 0) die("th4 pthread_join");
	printf("\n main() reporting that all threads have terminated successfuly\n\n");
	
	// Close udp sockets
	close(fd_server_sock); 
	close(fd_client_sock);

	// Close serial Ports
	close(fd_serial_1); 
	close(fd_serial_2);

	//De-allocates mutex resources
	pthread_mutex_destroy (&mutex1);
	pthread_mutex_destroy (&mutex2);

	//Free dynamic memory allocated
	free_mem(remote_server_ip_addr);
	
        return 0;
}

////////////////////////////////////////////////////////////////
int init_server_udp_socket(struct sockaddr_in *server_addr)
{
	int fd_sock;
	
	//Open socket       
        if ((fd_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        	die("socket");

	//Fill in server_addr structure
        server_addr->sin_family = AF_INET;
        server_addr->sin_port = htons(5000);
        server_addr->sin_addr.s_addr = INADDR_ANY;
        bzero(&(server_addr->sin_zero),8);

	//Bind socket to address
        if (bind(fd_sock,(struct sockaddr *)server_addr,
            sizeof(struct sockaddr)) == -1)
	        die("Bind");
		
	printf("\nEVB: UDPServer Waiting for client on port 5000\n\n");
        fflush(stdout);
	return fd_sock;
}

////////////////////////////////////////////////////////////////
int init_client_udp_socket(char *remote_server_ip_addr)
{

	int fd_sock;
	String str;
	struct hostent *host;
	char *p = NULL;

	//Open socket       
    if ((fd_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	    die("socket");
	
	//Get remote server address
	printf("remote_PC_ip_addr: ");
	scanf("%s",str);
	remote_server_ip_addr = (char*)malloc((strlen(str)+1));
	
	if(remote_server_ip_addr == NULL)
	{
		printf("Out of memory, exiting\n");
		exit(-1);
	}

	p = strcpy(remote_server_ip_addr,str);

	if (p == NULL)
		die("strcpy");

	//Get host info
	host= (struct hostent *)(intptr_t) gethostbyname((char *)remote_server_ip_addr);

	if (host == NULL)
		die("gethostbyname");
	
	//Fill in server_addr structure
        remote_server_addr.sin_family = AF_INET;
        remote_server_addr.sin_port = htons(5002);
	    remote_server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(remote_server_addr.sin_zero),8);


   	printf("\nEVB: UDP Client is initialized on port 5002\n\n");
        fflush(stdout);
	return fd_sock;
}
////////////////////////////////////////////////////////////////////
 int recv_data_udp_socket(void * fd_sock)
 {
 	int addr_len,rtn,s;
	ssize_t bytes_read;
	struct sockaddr_in client_addr;
        int sock;
	char * p = NULL;
	String local_rcv_buffer;
	sock = (intptr_t) fd_sock;
	addr_len = sizeof(struct sockaddr);

	while (1)
	{
        	bytes_read = recvfrom(sock,local_rcv_buffer,STR_SIZE,0, (struct sockaddr *)&client_addr, &addr_len);
		if (bytes_read == -1)
				die("recvfrom");

		if (bytes_read > 0)
		{
	 		local_rcv_buffer[bytes_read] = '\0';

			/* Atomicaly copy local buffer to global one */
			if ((rtn = pthread_mutex_lock(&mutex1)) != 0)
			{
	    			fprintf(stderr,"pthread_mutex_lock: %s",strerror(rtn));
	    			exit(-1);
			}

			//Read from udp socket to protected global buffer
			memset(sock_recv_buf,0,STR_SIZE);
			p = strncpy(sock_recv_buf,local_rcv_buffer,bytes_read+1);
			if (p == NULL)
				die("strncpy");

			s = pthread_cond_signal (&cond_var1);
			if (s != 0) 
			{
					fprintf(stderr,"Signal error!\n");
					exit(-1);
			}
			
			if ((rtn = pthread_mutex_unlock(&mutex1)) != 0)
			{
	    			fprintf(stderr,"pthread_mutex_unlock: %s",strerror(rtn));
	    			exit(-1);
			}

          		printf("recv_data_udp_socket():\t bytes_read=%d \t(%s , %d)\tsaid : ",bytes_read,inet_ntoa(client_addr.sin_addr),
                        	              		 ntohs(client_addr.sin_port));
         		printf("%s\n", local_rcv_buffer);

			s = fflush(stdout);
				if (s != 0) die("pthread_exit");

			if (strcmp(local_rcv_buffer,"Terminate program request has accepted!") == 0)
			{
				s = pthread_exit();
				if (s != 0) die("pthread_exit");
			}
		}
		sleep(1);
		memset(local_rcv_buffer,0,STR_SIZE);
        }
        return bytes_read;
}
//////////////////////////////////////////////////////////////////////
 int send_data_udp_socket(void * fd_sock)
 {
 	
    int rtn,s;
	struct sockaddr_in server_addr;
	String local_send_buffer, local_send_buffer_tmp;
	String * Plocal_send_buffer;
	int sock = (intptr_t) fd_sock;
	size_t send_len,to_send;
	ssize_t bytes_sent;
	char *p = NULL;

	Plocal_send_buffer = &local_send_buffer;

	while (1)
	{
		/* atomicly copy global buffer to local one */
		if ((rtn = pthread_mutex_lock(&mutex2)) != 0)
		{
	    	fprintf(stderr,"pthread_mutex_lock: %s",strerror(rtn));
	    	exit(-1);
		}

        while (!(send_len = strlen(uart_recv_buf)))
		{
			s = pthread_cond_wait(&cond_var2, &mutex2);
			if (s != 0) {fprintf(stderr,"Wait error!\n"); exit(-1);}
		}

		p = strncpy(local_send_buffer, uart_recv_buf, send_len);
		if (p == NULL)
			die("strncpy");

		memset(uart_recv_buf,0,STR_SIZE);

		if ((rtn = pthread_mutex_unlock(&mutex2)) != 0)
		{
	    		fprintf(stderr,"pthread_mutex_unlock: %s",strerror(rtn));
	    		exit(-1);
		}

		if (send_len > 0)
		{
			p = strncpy(local_send_buffer_tmp,local_send_buffer, send_len);
			if (p == NULL)
				die("strncpy");

			to_send=send_len;

			while(to_send)
			{
				bytes_sent=sendto(sock, local_send_buffer, strlen(local_send_buffer), 0,(struct sockaddr *)&remote_server_addr, sizeof(struct sockaddr));//For UDP
          			printf("send_data_udp_socket():\t bytes_sent=%d \t(%s , %d) \tsaid : %s\n\n",bytes_sent,inet_ntoa(remote_server_addr.sin_addr),ntohs(remote_server_addr.sin_port),local_send_buffer);
				Plocal_send_buffer += bytes_sent;
				to_send -= bytes_sent;
			}
			s = fflush(stdout);
				if (s != 0) die("pthread_exit");
		}

		if (strcmp(local_send_buffer_tmp,"Terminate program request has accepted!") == 0)
		{
			s = pthread_exit();
			if (s != 0) die("pthread_exit");
		}

		memset(local_send_buffer,0,STR_SIZE);
		memset(local_send_buffer_tmp,0,STR_SIZE);
		sleep(1);
	}
        return 0;
}
//////////////////////////////////////////////////////////////////////

int init_con_uart(int uart_num)
{
	int fd_serial;
  	struct termios options;
  	int n;
  	char path[255];
	char buff[255];
	
	switch (uart_num)
	{
		case 1:
			strcpy(path,"/dev/ttySAC1");
			break;	
		case 2:
			strcpy(path,"/dev/ttySAC2");
			break;
		default:
			strcpy(path,"/dev/nothing_there");
	}		

  	/*Open port */
  	fd_serial = open(path, O_RDWR | O_NOCTTY | O_NDELAY);  /* <--- YOUR PORT */

	if(fd_serial == -1) 
	{
	    	printf("ERROR Open Serial Port CON%d!\n",uart_num);
	        exit(-1);
  	}
  
  	/* Serial Configuration */
  	tcgetattr(fd_serial, &options);   // Get Current Config
  	cfsetispeed(&options, B9600); // Set Baud Rate
	cfsetospeed(&options, B9600);
	options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_iflag =  IGNBRK;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cflag |= CLOCAL | CREAD;
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 5;
    options.c_iflag &= ~(IXON|IXOFF|IXANY);
    options.c_cflag &= ~(PARENB | PARODD);
  
    /* Save The Configure */
    tcsetattr(fd_serial, TCSANOW, &options);
    tcflush(fd_serial,TCIOFLUSH);
        
    return fd_serial;
}
//////////////////////////////////////////////////////////////////////
int send_data_to_uart(void* fd_serial)
{
	int  sent_len=0;
	String local_uart_send_buf;
	int  length,rtn,s;
	int  fd = (intptr_t) fd_serial;
	char *p = NULL;

	while(1)
	{
		/* Atomicly copy global buffer to local one */
		if ((rtn = pthread_mutex_lock(&mutex1)) != 0)
		{
	    		fprintf(stderr,"pthread_mutex_lock: %s",strerror(rtn));
	    		exit(-1);
		}

		while (!(length = strlen(sock_recv_buf)))
		{
			s = pthread_cond_wait(&cond_var1, &mutex1);
				if (s != 0) {fprintf(stderr,"Wait error!\n"); exit(-1);}
		}

		p = strncpy(local_uart_send_buf,sock_recv_buf,length+1);
		if (p == NULL)
			die("strncpy");

		memset(sock_recv_buf,0,STR_SIZE);

		if ((rtn = pthread_mutex_unlock(&mutex1)) != 0)
		{
	    		fprintf(stderr,"pthread_mutex_unlock: %s",strerror(rtn));
	    		exit(-1);
		}

		if (length>0)
		{
			printf("send_data_to_uart():\t length=%d \t\t\t\tsaid : %s\n", length,local_uart_send_buf);
			write(fd,local_uart_send_buf,length);
		}

		if (strcmp(local_uart_send_buf,"Terminate program request has accepted!") == 0)
		{
			s = pthread_exit();
			if (s != 0) die("pthread_exit");
		}
		memset(local_uart_send_buf,0,STR_SIZE);
		sleep(1);
	}
    	return 0;
}
//////////////////////////////////////////////////////////////////////
int read_data_from_uart(void* fd_serial)
{
    int read_len,rtn,s;
	String local_uart_recv_buf;
	int  fd = (intptr_t) fd_serial;
	char * p;

	while(1)
	{
	    read_len = read(fd,local_uart_recv_buf,STR_SIZE);   // Read Data From Serial Port
    	local_uart_recv_buf[read_len] = '\0';
    	

   		if(read_len>0)
    	{
	      	printf("read_data_from_uart():\t read_len=%d \t\t\t\tsaid : %s\n",
	      	read_len,
	      	local_uart_recv_buf); // Print Out

			/* Atomicly copy global buffer to local one */
			if ((rtn = pthread_mutex_lock(&mutex2)) != 0)
			{
	    			fprintf(stderr,"pthread_mutex_lock: %s",strerror(rtn));
	    			exit(-1);
			}

			p = strncpy(uart_recv_buf,local_uart_recv_buf,read_len+1);			
			if (p == NULL)
				die("strncpy");

			s = pthread_cond_signal (&cond_var2);
			if (s != 0) 
			{
					fprintf(stderr,"Signal error!\n");
					exit(-1);
			}

			if ((rtn = pthread_mutex_unlock(&mutex2)) != 0)
			{
	    			fprintf(stderr,"pthread_mutex_unlock: %s",strerror(rtn));
	    			exit(-1);
			}
	    }

		if (strcmp(local_uart_recv_buf,"Terminate program request has accepted!") == 0)
		{
			s = pthread_exit();
			if (s != 0) die("pthread_exit");
		}

		memset(local_uart_recv_buf,0,STR_SIZE);
		sleep(1);
	}
     	return 0;
}

  
