/*
 * router_message.h : This header file represents a message structure as specified in the specs
 * to be passed between different nodes for distance vector updation.
 * 
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */

#ifndef ROUTER_MESSAGE_H
#define ROUTER_MESSAGE_H

#include <stdint.h>

/*
 * Structure to represent one update field in the message.
 * The format is as per the specification in project document.
 */ 
struct distance_vector_entry {
	uint32_t server_ip;			// IP of the server in sender's routing table
	uint16_t server_port;		// Port of the server in sender's routing table
	uint16_t filler;			// Zero filler
	uint16_t server_id;			// Id of the server in sender's routing table.
	uint16_t dist;				// Cost of the path to server_id from the sender.
};

/*
 * This structure represents a message to be passed between the routers.
 * The format is as per the specification in project document.
 */ 
struct message {
	uint16_t update_fields;		// Number of update fields
	uint16_t server_port;		// Port of the server sending this packet
	uint32_t server_ip;			// IP Address of the server sending this packet
	struct distance_vector_entry dv_entry[];	// Array of update fields
};
#endif
