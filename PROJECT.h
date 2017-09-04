/* 
Project - Rs232 and Ethernet Loop 
File name: PROJECT.h


This file is part of a final project for Embedded Linux Course

The project requires 5 files:
PC_server.c
PC_client.c
evb2.c
PROJECT.h (THIS ONE)
EVB2.h
*/

#ifndef PROJECT_H
#define PROJECT_H

////////////////////////////* Includes */////////////////////

#include <sys/types.h>	/*Data types*/
#include <sys/socket.h>	/*Core socket functions and data structures*/
#include <netinet/in.h>	/*constants and structures for internet domain addresses*/
#include <arpa/inet.h>	/*definitions for internet operations*/
#include <stdio.h>	/*C file input/output functions*/
#include <unistd.h>	/*access to the POSIX operating system API*/
#include <errno.h>	/*macros for reporting and retrieving error conditions*/
#include <string.h>	/*operations on strings*/
#include <stdlib.h>	/*strings,math,I/O,OS services*/
#include  <sys/ioctl.h> /* ioctl() */

////////////////////////////* Macros *//////////////////////////

#define STR_SIZE 1024

////////////////////////////* Types *//////////////////////////

typedef char String[STR_SIZE];

////////////////////////////* Prototypes */////////////////////

void die(const char *msg);
void free_mem(char *mem);

////////////////////////////* Globals *///////////////////////

void die(const char *msg)
{
	perror(msg);
	exit(1);
}

void free_mem(char *mem)
{
	free(mem);
	mem = NULL;
}

#endif /*PROJECT_H*/
