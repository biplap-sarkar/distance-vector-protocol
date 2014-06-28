/*
 * topology.h : Header file for functions to represent a network
 * topology
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <limits.h>
#include <time.h>
#include "router_message.h"

#define INFINITY 32767		// Defining the max value of 16 bit signed integer ( 2^15 - 1) as infinity
#define IPLENGTH 20

/*
 * Defines a node in the topology
 */
struct node {
	int id;		// Id of the node
	char *ip;	// IP address of the node
	int port;	// Port at which the node is listening
	struct node *next;	// Pointer to next node in the global node list
};


/*
 * This structure defines one entry in the distance vector.
 * A list of this structure represent one distance vector.
 */ 
struct distance_vector {
	int node_id;	// destination node id
	int dist;		// distance
	struct distance_vector *next;	// Pointer to next entry in the distance vector
};


/*
 * This structure represents one entry in the forwarding (routing) distance vector.
 * A list of this structure represent the forwarding (routing) distance vector.
 */ 
struct routing_distance_vector {
	int to_node;	// destination node id
	int dist;		// estimated distance
	int via_node;	// node id of next hop
	struct routing_distance_vector *next;	// Pointer to next entry in the routing distance vector
};


/*
 * This structure represent one entry in the distance matrix.
 * A list of this structure represents the distance matrix and keeps the distance vectors
 * passed by all the neighbours
 */ 
struct distance_matrix {
	int from_node;		// Node id of the neighbour from which this distance vector is populated
	int cost;			// Direct link cost to the neighbour
	struct timespec last_updated;	// Time stamp of the last received update
	struct distance_vector *to;		// Distance vector sent by the neighbour
	struct distance_matrix *next;	// Pointer to next entry in the distance matrix
};


/*
 * Creates a new node in the global node list.
 * 
 * @args id: node id of the new node
 * @args ip: IP address of the new node
 * @args port: port at which the new node will listen
 */ 
void create_new_node(int id, char *ip, int port);

/*
 * This function initializes the distance matrix
 * 
 * @args neighbours: pointer to list of neighbours
 * @args cost: pointer to list of costs to neighbours
 * @args count: number of neighbours
 */
void init_distance_matrix(int *neighbours, int *cost, int count);

/*
 * This function initializes a distance vector of a neighbour
 * 
 * @args from_node: node id of the neighbour where the updates from the neighbour will be kept
 */ 
struct distance_vector * create_distance_vector(int from_node);

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
int set_distance_matrix_value(int from_node, int to_node, int cost);

/*
 * Gets the distance from distance vector of a given neighbour to a
 * given node.
 * 
 * @args from_node: node id of the neighbour to which this distance vector belongs
 * @args to_node: node id of the destination.
 */ 
int get_distance_matrix_value(int from_node, int to_node);

/*
 * Gets the information about a node from it's node id
 * 
 * @args id: node id
 */ 
struct node * get_node(int id);

/*
 * Gets the node id of a node from it's IP and port
 * 
 * @args ip: IP of the node
 * @args port: port of the node
 */
int get_nodeid(char *ip, int port);

/*
 * This function initializes the routing distance vector
 * ie the vector which will keep the shortest estimated distance to all the nodes
 */ 
void initialize_distance_matrix();

/*
 * Deletes a neighbour from the distance matrix.
 * 
 * @args node_id: node id of the neighbour to be deleted
 * 
 * @return: node id of the deleted neighbour, -1 if node_id did not belong to any neighbour
 */
int delete_neighbour(int node_id);

/*
 * Checks if a given node id is neighbour of this node or not
 * 
 * @args node_id : node id of the neighbour
 * @returns: 1 if the node_id is neighbour of this node, 0 otherwise
 */ 
int is_neighbour(int node_id);

#endif
