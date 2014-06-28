/*
 * router_simulator.h : Header file for functions to simulator the functions of
 * a router and handle UI commands.
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */

#ifndef ROUTER_SIMULATOR_H
#define ROUTER_SIMULATOR_H
#include "router_message.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define BUFFLEN 1024

#define BILLION 1E9				
#define MAX_UPDATE_TIMEOUT 3		// update interval for sending distance vector to neighbours

/*
 * This function simulates a node crash.
 */
void crash();

/*
 * This function sets the routing distance to a given node
 * 
 * @arg to: node id of the destination
 * @arg dist: new distance
 */
void set_routing_distance(int to, int dist);

/*
 * Builds a message representing distance vector of this node
 * which can be sent to the neigbours of this node.
 * 
 * @return message: Message representing the distance vector of this node
 */ 
struct message * build_message();

/*
 * Initializes the topology and distance vector
 * 
 * @args topology_file: Topology file
 */
void init_topology(char *topology_file);

/*
 * This function returns shortest distance information to a given destination.
 * 
 * @arg to: destination node id
 * @returns : a pointer of type struct routing_distance_vector containing destination id,
 * estimated distance and next hop information.
 */
struct routing_distance_vector * get_routing_vector(int to);

/*
 * This function multiplexes processing of UI commands,
 * incoming router updates and select timeout.
 */
void process_command();

/*
 * This function processes a UI command.
 */
void process_ui_command();

/*
 * Receives a distance vector update from a neighbour
 * 
 * @returns: pointer to struct message which contains all the
 * details regarding a message as specified in the specificaiton. 
 */
struct message * recv_msg();

/*
 * This function sends the distance vector of this node to all
 * it's neighbours.
 */
void send_dv_to_neighbours();

/*
 * This function sends a message to a specific node
 * 
 * @args msg: message to be sent
 * @args ip: IP of the node where the message is to be sent
 * @args port: port of the node where the message is to be sent
 */ 
void send_msg(struct message *msg, char *ip, int port);

/*
 * Sets the cost to a neighbour
 * 
 * @args nbr : node id of the neighbour
 * @args newcost : new cost to reach the neighbour in direct link
 */
void set_neighbour_cost(int nbr, int cost);

/*
 * Starts listening for Datagram messages
 * 
 * @return: 0 in case of success, exits the program if fails
 */
int setup_listener();

/*
 * This function displays the forwarding table of this node
 * in sorted order of node id
 */
void display_routing_table();

/*
 * Updates the distance vector of a particular node in the distance matrix
 * based upon received message.
 * 
 * @args msg: Message received from a neigbour
 */ 
int update_distance_vector(struct message *msg);


/*
 * Implements Bellman Ford algorithm to compute the shortest distance vector
 * to all the nodes from the available distance matrix.
 */ 
void update_routing_distance_vector();

/*
 * This function checks the distance matrix to see if there is any neighbour
 * from which no updates has been recieved for 3*timeout units of time.
 * If there is any such node, this node marks that neighbour as unreachable
 * and set it's cost to infinity 
 */ 
void update_unreachable_node_cost();

/*
 * This function resets the routing table
 */ 
void reset_routing_distance_vector();

#endif
