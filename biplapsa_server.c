/*
 * biplapsa_server.c : This file is the main entry to this application.
 * It reads the command options from the command promt and starts the application
 * 
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "router_simulator.h"

//void process_command();
extern int update_interval;

int main(int argc, char **argv){
	if(argc != 5){
		printf("Usage:- server -t <topology-file-name> -i <routing-update-interval>\n");
		return -1;
	}
	char option[10];
	char topology_file[BUFFLEN];
	memset(topology_file, 0, BUFFLEN);
	memset(option, 0, 10);
	int i;
	for(i=0;i<=1;i++){
		if(strcmp(argv[(i*2) + 1], "-i") == 0){
			char *endptr;
			update_interval = strtol(argv[(i*2)+2], &endptr, 10);
			if(errno != 0 || strlen(endptr)>0){
				printf("Invalid update interval\n");
				return -1;
			}
			if(update_interval <= 0){
				printf("Update interval has to be a non zero positive value\n");
				return -1;
			}
		}
		
		else if(strcmp(argv[(i*2) + 1], "-t") == 0){
			strcpy(topology_file, argv[(i*2) + 2]);
		}
		else{
			printf("Invalid option %s\n",argv[(i*2) + 1]);
			return -1;
		}
	}
	init_topology(topology_file);
	setup_listener();
	setbuf(stdout, NULL);
	process_command();
}
