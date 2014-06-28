/*
 * file_utils.c : Contains functions to read and parse the topology file
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "file_utils.h"
#include "topology.h"

#define IPLENGTH 20

extern char my_ip[IPLENGTH];		// IP of this node
extern int my_id;		// node id of this node
extern int my_port;		// port at which this application is listening for incoming messages

/*
 * This function reads and parses the topology file
 * 
 * @args filename: topology file name
 * 
 * @returns : 0 in case of success, -1 otherwise
 * 
 */
int read_topology(char *filename){
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL){
		printf("Cannot open file %s\n",filename);
		return -1;
	}
	int nd_count, nb_count;
	fscanf(fp, "%d",&nd_count);
	fscanf(fp, "%d",&nb_count);
	int i;
	char ip[IPLENGTH];
	int id, port;
	for(i=0;i<nd_count;i++){
		memset(ip,0,IPLENGTH);
		fscanf(fp,"%d %s %d", &id, ip, &port);
		create_new_node(id, ip, port);
	}
	int *dest = (int *)malloc(sizeof(int)*nb_count);
	int *cost = (int *)malloc(sizeof(int)*nb_count);
	for(i=0;i<nb_count;i++){
		fscanf(fp, "%d %d %d",&id, &dest[i], &cost[i]);
		if(my_id == -1){
			my_id = id;
			memset(my_ip, 0, IPLENGTH);
			struct node *my_node = get_node(my_id);
			if(my_node == NULL){
				printf("Could not find any information about self node in topology, make sure topology file is correct\n");
				return -1;
			}
			strcpy(my_ip, my_node->ip);
			my_port = my_node->port;
		}
	}
	init_distance_matrix(dest, cost, nb_count);
	free(dest);
	free(cost);
	fclose(fp);
	return 0;
}

