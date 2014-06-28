/*
 * topology.c : Contains functions to represent a network
 * topology
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include "topology.h"

struct node *node_list = NULL;		// global node list keeping information of all the nodes
struct distance_matrix *dist_matrix = NULL;		// distance matrix keeping distance vectors from all neighbours
struct routing_distance_vector *rt_distance_vector = NULL;		// distance vector of the forwarding table

int node_count = 0;		// total nodes in the topology

int my_id = -1;		// node id of this node

char my_ip[IPLENGTH];		// IP of this node

/*
 * Creates a new node in the global node list.
 * 
 * @args id: node id of the new node
 * @args ip: IP address of the new node
 * @args port: port at which the new node will listen
 */ 
void create_new_node(int id, char *ip, int port){
	struct node *new_node = (struct node *)malloc(sizeof(struct node));
	new_node->ip = (char *)malloc(sizeof(char)*IPLENGTH);
	memset(new_node->ip, 0, IPLENGTH);
	
	new_node->id = id;
	strcpy(new_node->ip, ip);
	new_node->port = port;
	new_node->next = node_list;
	
	node_list = new_node;
	node_count++;
}

/*
 * Gets the information about a node from it's node id
 * 
 * @args id: node id
 */ 
struct node * get_node(int id){
	struct node *nd = node_list;
	while(nd){
		if(nd->id == id)
			return nd;
		nd = nd->next;
	}
	return NULL;
}

/*
 * This function initializes the routing distance vector
 * ie the vector which will keep the shortest estimated distance to all the nodes
 */ 
void init_routing_distance_vector(){
	struct node *nd = node_list;
	while(nd){
		struct routing_distance_vector *r_dv = (struct routing_distance_vector *)malloc(sizeof(struct routing_distance_vector));
		r_dv->next = rt_distance_vector;
		r_dv->to_node = nd->id;
		if(r_dv->to_node == my_id)
			r_dv->dist = 0;
		else
			r_dv->dist = INFINITY;
		r_dv->via_node = nd->id;
		rt_distance_vector = r_dv;
		nd = nd->next;
	}
}


/*
 * This function initializes the distance matrix
 * 
 * @args neighbours: pointer to list of neighbours
 * @args cost: pointer to list of costs to neighbours
 * @args count: number of neighbours
 */ 
void init_distance_matrix(int *neighbours, int *cost, int count){
	int i=0;
	for(i=0;i<count;i++){
		struct distance_matrix *new_dm = (struct distance_matrix *)malloc(sizeof(struct distance_matrix));
		new_dm->from_node = neighbours[i];
		new_dm->cost = cost[i];
		new_dm->to = create_distance_vector(neighbours[i]);
		clock_gettime(CLOCK_REALTIME, &new_dm->last_updated);
		new_dm->next = dist_matrix;
		dist_matrix = new_dm;
	}
}

/*
 * This function initializes a distance vector of a neighbour
 * 
 * @args from_node: node id of the neighbour where the updates from the neighbour will be kept
 */ 
struct distance_vector * create_distance_vector(int from_node){
	struct distance_vector *new_dv_head = NULL;
	struct node *nd = node_list;
	while(nd){
		struct distance_vector *new_dv = (struct distance_vector *)malloc(sizeof(struct distance_vector));
		new_dv->node_id = nd->id;
		new_dv->next = new_dv_head;
		if(new_dv->node_id == from_node)
			new_dv->dist = 0;
		else
			new_dv->dist = INFINITY;
		new_dv_head = new_dv;
		nd = nd->next;
	}
	return new_dv_head;
}

/*
 * Sets the distance vector value from a given neighbour to a given node.
 * 
 * @args from_node: node id of the neighbour to which this distance vector belongs
 * @args to_node: destination node id
 * @args dist: new distance
 * 
 * @return: node id of the neighbour whose distance vector is getting updated, -1 in case the from_node
 * does not belong to any neighbour
 */
int set_distance_matrix_value(int from_node, int to_node, int dist){
	struct distance_matrix *dm = dist_matrix;
	struct distance_vector *dv;
	while(dm){
		if(dm->from_node == from_node){
			dv = dm->to;
			while(dv){
				if(dv->node_id == to_node){
					dv->dist = dist;
					break;
				}
				dv = dv->next;
			}
			clock_gettime(CLOCK_REALTIME, &dm->last_updated);
			return dm->from_node;
		}
		dm = dm->next;
	}
	return -1;
}

/*
 * Gets the distance from distance vector of a given neighbour to a
 * given node.
 * 
 * @args from_node: node id of the neighbour to which this distance vector belongs
 * @args to_node: node id of the destination.
 */ 
int get_distance_matrix_value(int from_node, int to_node){
	struct distance_matrix *dm = dist_matrix;
	struct distance_vector *dv;
	while(dm){
		if(dm->from_node == from_node){
			dv = dm->to;
			while(dv){
				if(dv->node_id == to_node)
					return dv->dist;
				dv = dv->next;
			}
		}
		dm = dm->next;
	}
	return INFINITY;
}

/*
 * Deletes a neighbour from the distance matrix.
 * 
 * @args node_id: node id of the neighbour to be deleted
 * 
 * @return: node id of the deleted neighbour, -1 if node_id did not belong to any neighbour
 */ 
int delete_neighbour(int node_id){
	struct distance_matrix *dm = dist_matrix;
	struct distance_vector *dv,*tmp;
	if(dm == NULL)
		return -1;
	if(dm->from_node == node_id){
		dv = dm->to;
		while(dv){
			tmp = dv;
			dv = dv->next;
			free(tmp);
		}
		dist_matrix = dist_matrix->next;
		free(dm);
		return node_id;
	}
	while(dm->next){
		if(dm->next->from_node == node_id)
			break;
		dm = dm->next;
	}
	if(dm->next){
		dv = dm->next->to;
		while(dv){
			tmp = dv;
			dv = dv->next;
			free(tmp);
		}
		struct distance_matrix *tmp_dm = dm->next;
		dm->next = tmp_dm->next;
		free(tmp_dm);
		return node_id;
	}
	return -1;
}

/*
 * Gets the node id of a node from it's IP and port
 * 
 * @args ip: IP of the node
 * @args port: port of the node
 */
int get_nodeid(char *ip, int port){
	struct node *nd = node_list;
	while(nd){
		if(strcmp(nd->ip, ip)==0 && nd->port == port)
			return nd->id;
		nd = nd->next;
	}
	return -1;
}

/*
 * Checks if a given node id is neighbour of this node or not
 * 
 * @args node_id : node id of the neighbour
 * @returns: 1 if the node_id is neighbour of this node, 0 otherwise
 */ 
int is_neighbour(int node_id){
	struct distance_matrix *dm = dist_matrix;
	while(dm){
		if(dm->from_node == node_id)
			return 1;
		dm = dm->next;
	}
	return 0;
}
