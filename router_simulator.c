/*
 * router_simulator.c : Contains functions to simulator the functions of
 * a router and handle UI commands.
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "router_simulator.h"
#include "topology.h"


int sockd;	// socket descriptor for listening to incoming messages
int maxfd;	// max fd used in select
int my_port;	// port at which this application is listening for incoming messages
struct sockaddr_in serv_addr;	// contains description of the listening socket
int update_interval;	// update interval in seconds for sending distance vector to neighbours
fd_set fds;		// file desctriptor set to be used in select
int packets_received = 0;	// packets received by this application


extern struct node *node_list;		// global node list keeping information of all the nodes
extern struct routing_distance_vector *rt_distance_vector;	// distance vector of the forwarding table
extern struct distance_matrix *dist_matrix;	// distance matrix keeping distance vectors from all neighbours


extern int node_count;		// total nodes in the topology

extern int my_id;		// node id of this node
extern char my_ip[IPLENGTH];	// IP of this node

struct timeval timeout;		// timeval to keep the remaining timeout time


/*
 * Starts listening for Datagram messages
 * 
 * @return: 0 in case of success, exits the program if fails
 */
int setup_listener(){
	if((sockd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		perror("Couldn't create socket to listen\n");
		exit(1);
	}
	memset((void *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(my_port);
	if (bind(sockd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Socket bind error\n");
		exit(1);
	}
	return 0;
}

/*
 * This function multiplexes processing of UI commands,
 * incoming router updates and select timeout.
 */ 
void process_command(){
	int fdi;
	fd_set tfds;
	maxfd = sockd;
	
	FD_ZERO(&fds);
	FD_SET(STDIN, &fds);
	FD_SET(sockd, &fds);
	maxfd = sockd;
	int time_elapsed;
	int prompt = 1;
	timeout.tv_sec = update_interval;
	timeout.tv_usec = 0;
	update_routing_distance_vector();
	while(1){
		if(prompt){
			printf("router_simulator>");
			prompt = 0;
		}
		FD_ZERO(&tfds);
		tfds = fds;
		if(timeout.tv_sec == 0){
			timeout.tv_sec = update_interval;
			timeout.tv_usec = 0;
		}
		select(maxfd+1, &tfds, NULL, NULL, &timeout);
		update_unreachable_node_cost();
		time_elapsed = 1;
		if(FD_ISSET(STDIN, &tfds)){
			time_elapsed = 0;
			prompt = 1;
			process_ui_command();
		}
		if(FD_ISSET(sockd, &tfds)){
			time_elapsed = 0;
			struct message *msg = recv_msg();
			int sender = update_distance_vector(msg);
			if(sender != -1){
				packets_received = packets_received + 1;
				prompt = 1;
				update_routing_distance_vector();
				printf("RECEIVED A MESSAGE FROM SERVER %d\n",sender);
			}
			free(msg);
		}
		if(time_elapsed) {			// timer elapsed, send distance vector to neighbours
			send_dv_to_neighbours();
		}
	}
}

/*
 * This function processes a UI command.
 */
void process_ui_command(){
	char *buff = (char *)malloc(sizeof(char)*BUFFLEN);
	memset(buff, 0, BUFFLEN);
	fgets(buff, BUFFLEN-1, stdin);
	int len = strlen(buff);
	if(len == 0)
		return;
	buff[len-1] = '\0';
	char *cmd = strtok(buff," ");
	if(cmd == NULL){
		return;
	}
	if(strcmp(cmd,"update")==0){	// handle update command
		char *endptr;
		int serv_id1, serv_id2;
		char *server_id1 = strtok(NULL," ");
		
		if(server_id1 == NULL){
			printf("update : Usage: update <server_id1> <server_id2> <link cost>\n");
			free(buff);
			return;
		}
		serv_id1 = strtol(server_id1, &endptr, 10);
		if(errno != 0 || strlen(endptr) > 0){
			printf("update : Error: Invalid server id\n");
			free(buff);
			return;
		}
		
		if(serv_id1 != my_id){
			printf("update : Error: Source node id has to be id of this machine\n");
			free(buff);
			return;
		}
		
		char *server_id2 = strtok(NULL, " ");
		if(server_id2 == NULL){
			printf("update : Usage: update <server_id1> <server_id2> <link cost>\n");
			free(buff);
			return;
		}
		serv_id2 = strtol(server_id2, &endptr, 10);
		if(errno != 0 || strlen(endptr) > 0){
			printf("update : Error: Invalid server id\n");
			free(buff);
			return;
		}
		if(is_neighbour(serv_id2) == 0){
			printf("update : error, %d is not a neighbour of this node\n",serv_id2);
			free(buff);
			return;
		}
		char *_lc = strtok(NULL, " ");
		if(_lc == NULL){
			printf("update : Usage: update <server_id1> <server_id2> <link cost>\n");
			free(buff);
			return;
		}
		if(strcasecmp(_lc,"inf")==0){
			set_neighbour_cost(serv_id2, INFINITY);
			printf("update SUCCESS\n");
			printf("Link cost updated\n");
			free(buff);
			return;
		}
		int link_cost = strtol(_lc, &endptr, 10);
		if(errno != 0 || strlen(endptr) != 0){
			printf("update : Invalid link cost\n");
			free(buff);
			return;
		}
		set_neighbour_cost(serv_id2, link_cost);
		update_routing_distance_vector();
		printf("update SUCCESS\n");
		printf("Link cost updated\n");
	}
	else if(strcasecmp(cmd,"step")==0){		// handle step command
		send_dv_to_neighbours();
		printf("%s SUCCESS\n",cmd);
		timeout.tv_sec = update_interval;
		timeout.tv_usec = 0;
	}
	else if(strcasecmp(cmd,"display")==0){	// handle display command
		printf("destination-server-id\tnext-hop-server-Id\tcost-of-path\n");
		display_routing_table();
		printf("%s SUCCESS\n",cmd);
	}
	else if(strcasecmp(cmd,"disable")==0){	// handle disable command
		char *endptr;
		int serv_id;
		char *server_id = strtok(NULL," ");
		
		if(server_id == NULL){
			printf("disable : Usage: disable <server_id> \n");
			free(buff);
			return;
		}
		serv_id = strtol(server_id, &endptr, 10);
		if(errno != 0 || strlen(endptr) > 0){
			printf("disable : Error: Invalid server id\n");
			free(buff);
			return;
		}
		int res = delete_neighbour(serv_id);
		if(res >= 0){
			update_routing_distance_vector();
			printf("disable SUCCESS\n");
		}
		else{
			printf("disable: %d is not a neighbour\n",serv_id);
		}
	}
	else if(strcasecmp(cmd,"packets")==0){		// handle packets command
		printf("%d packets received by this server since last invocation\n",packets_received);
		printf("packets SUCCESS\n");
		packets_received = 0;
	}
	else if(strcasecmp(cmd,"crash")==0){		// handle crash command
		crash();
	}
	else{										// invalid command
		printf("Invalid command %s\n",cmd);
	}
	free(buff);
}

/*
 * This function displays the forwarding table of this node
 * in sorted order of node id
 */
void display_routing_table(){
	struct routing_distance_vector *r_dv = rt_distance_vector;
	int min_printed = 0;
	int i=0,j=0;
	for(i=0;i<node_count;i++){
		r_dv = rt_distance_vector;
		int min_toprint = r_dv->to_node;
		for(j=0;j<node_count;j++){
			if(r_dv->to_node <= min_printed){
				r_dv = r_dv->next;
				if(min_toprint <= min_printed)
					min_toprint = r_dv->to_node;
			}
			else {
				if(r_dv->to_node < min_toprint)
					min_toprint = r_dv->to_node;
				r_dv = r_dv->next;
			}
		}
		r_dv = rt_distance_vector;
		while(r_dv){
			if(r_dv->to_node == min_toprint){
				if(r_dv->dist == INFINITY)
					printf("%d\t\t\tNA\t\t\tinfinity\n",r_dv->to_node);
				else
					printf("%d\t\t\t%d\t\t\t%d\n",r_dv->to_node, r_dv->via_node, r_dv->dist);
			}
			r_dv = r_dv->next;
		}
		min_printed = min_toprint;
	}
	
}

/*
 * This function simulates a node crash.
 */
void crash(){
	while(1){
		sleep(update_interval*1000);	// A simple while loop was consuming lot of CPU, so better to go to sleep for a long time interval
	}
}

/*
 * This function sends the distance vector of this node to all
 * it's neighbours.
 */
void send_dv_to_neighbours(){
	struct distance_matrix *dm = dist_matrix;
	struct node *nd;
	struct message *msg = build_message();
	while(dm){
		nd = get_node(dm->from_node);
		send_msg(msg, nd->ip, nd->port);
		dm = dm->next;
	}
	free(msg);
}

/*
 * Receives a distance vector update from a neighbour
 * 
 * @returns: pointer to struct message which contains all the
 * details regarding a message as specified in the specificaiton. 
 */
struct message * recv_msg(){
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	int msglen = sizeof(struct message) + sizeof(struct distance_vector_entry)*node_count;
	struct message *msg = (struct message *)malloc(msglen);
	recvfrom(sockd, (void *)msg, msglen, 0, (struct sockaddr *)&client_addr, &len);
	return msg; 
}

/*
 * This function sends a message to a specific node
 * 
 * @args msg: message to be sent
 * @args ip: IP of the node where the message is to be sent
 * @args port: port of the node where the message is to be sent
 */ 
void send_msg(struct message *msg, char *ip, int port){
	int rem_sockd;
	struct sockaddr_in rem_addr;
	socklen_t len;
	int msglen = sizeof(struct message) + sizeof(struct distance_vector_entry)*node_count;
	if((rem_sockd = socket(AF_INET, SOCK_DGRAM, 0))<0){
		perror("Couldn't create socket to listen\n");
		exit(1);
	}
	memset((void *)&rem_addr, 0, sizeof(rem_addr));
	rem_addr.sin_family = AF_INET;
	inet_pton(AF_INET,ip,&rem_addr.sin_addr);
	rem_addr.sin_port = htons(port);
	sendto(rem_sockd, (void *)msg, msglen, 0, (struct sockaddr *)&rem_addr, sizeof(rem_addr));
	close(rem_sockd);
}

/*
 * Initializes the topology and distance vector
 * 
 * @args topology_file: Topology file
 */
void init_topology(char *topology_file){
	read_topology(topology_file);
	init_routing_distance_vector();
}

/*
 * Builds a message representing distance vector of this node
 * which can be sent to the neigbours of this node.
 * 
 * @return message: Message representing the distance vector of this node
 */ 
struct message * build_message(){
	struct message *msg = (struct message *)malloc(sizeof(struct message) + sizeof(struct distance_vector_entry)*node_count);
	msg->update_fields = (int16_t) node_count;
	msg->server_port = (int16_t) my_port;
	struct sockaddr_in addr;
	inet_pton(AF_INET, my_ip, &(addr.sin_addr));
	msg->server_ip = addr.sin_addr.s_addr;
	int i=0;
	struct routing_distance_vector *dv = rt_distance_vector;
	while(dv){
		struct sockaddr_in node_addr;
		int dist = dv->dist;
		struct node *nd = get_node(dv->to_node);
		inet_pton(AF_INET, nd->ip, &(node_addr.sin_addr));
		msg->dv_entry[i].server_ip = node_addr.sin_addr.s_addr;
		msg->dv_entry[i].server_port = nd->port;
		msg->dv_entry[i].server_id = nd->id;
		msg->dv_entry[i].filler = 0;
		msg->dv_entry[i].dist = dist;
		
		dv = dv->next;
		i = i+1;
	}
	return msg;
}

/*
 * Updates the distance vector of a particular node in the distance matrix
 * based upon received message.
 * 
 * @args msg: Message received from a neigbour
 */ 
int update_distance_vector(struct message *msg){
	struct distance_matrix *dm = dist_matrix;
	char sender[IPLENGTH];
	struct sockaddr_in sender_addr;
	memset(sender, 0, IPLENGTH);
	
	sender_addr.sin_addr.s_addr = msg->server_ip;
	inet_ntop(AF_INET, &(sender_addr.sin_addr), sender, INET_ADDRSTRLEN);
	int from_node = get_nodeid(sender, msg->server_port);
	int i=0;
	for(i=0;i<node_count;i++){
		int res = set_distance_matrix_value(from_node, msg->dv_entry[i].server_id, msg->dv_entry[i].dist);	// update the distance vector of the specified neighbour
		if(res == -1)
			return res;
	}
	return from_node;
}

/*
 * This function sets the routing distance to a given node
 * 
 * @arg to: node id of the destination
 * @arg dist: new distance
 */ 
void set_routing_distance(int to, int dist){
	struct routing_distance_vector *r_dv = rt_distance_vector;
	while(r_dv){
		if(r_dv->to_node == to){
			r_dv->dist = dist;
			r_dv->via_node = my_id;
			printf("Set distance to %d as %d via %d\n",r_dv->to_node, r_dv->dist, r_dv->via_node);
		}
		r_dv = r_dv->next;
	}
}

/*
 * Implements Bellman Ford algorithm to compute the shortest distance vector
 * to all the nodes from the available distance matrix.
 */ 
void update_routing_distance_vector(){
	struct routing_distance_vector *r_dv = rt_distance_vector;
	struct distance_matrix *dm = NULL;
	reset_routing_distance_vector();
	while(r_dv){						// estimate new min distance to all the nodes 
		int to_node = r_dv->to_node;
		int old_dist = r_dv->dist;
		int from_node = r_dv->via_node;
		dm = dist_matrix;
		while(dm){						// go through each distance vector in the distance matrix
			int from_nbr = dm->from_node;
			int new_dist = dm->cost + get_distance_matrix_value(from_nbr, to_node);	// estimated distance from neighbour dm->from_node
			if(new_dist < old_dist){	// estimated distance is less than previous routing distance
				old_dist = new_dist;	// update the routing distance
				from_node = from_nbr;	// update the from_node
			}
			dm = dm->next;
		}
		r_dv->dist = old_dist;
		r_dv->via_node = from_node;
		
		r_dv = r_dv->next;
	}
}

/*
 * This function returns shortest distance information to a given destination.
 * 
 * @arg to: destination node id
 * @returns : a pointer of type struct routing_distance_vector containing destination id,
 * estimated distance and next hop information.
 */ 
struct routing_distance_vector * get_routing_vector(int to){
	struct routing_distance_vector *r_dv = rt_distance_vector;
	while(r_dv){
		if(r_dv->to_node == to)
			return r_dv;
		r_dv = r_dv->next;
	}
	return NULL;
}

/*
 * Sets the cost to a neighbour
 * 
 * @args nbr : node id of the neighbour
 * @args newcost : new cost to reach the neighbour in direct link
 */
void set_neighbour_cost(int nbr, int newcost){
	struct distance_matrix *dm = dist_matrix;
	while(dm){
		if(dm->from_node == nbr)
			break;
		dm = dm->next;
	}
	if(dm){
		int oldcost = dm->cost;
		struct distance_vector *dv = dm->to;
		while(dv){
			dv->dist = dv->dist - oldcost + newcost;
			dv = dv->next;
		}
		
		struct routing_distance_vector *r_dv = rt_distance_vector;
		while(r_dv){
			if(r_dv->via_node == nbr)
				r_dv->dist = r_dv->dist - oldcost + newcost;
			r_dv = r_dv->next;
		}
		dm->cost = newcost;
	}
}

/*
 * This function checks the distance matrix to see if there is any neighbour
 * from which no updates has been recieved for 3*timeout units of time.
 * If there is any such node, this node marks that neighbour as unreachable
 * and set it's cost to infinity 
 */ 
void update_unreachable_node_cost(){
	struct timespec current_time;
	clock_gettime(CLOCK_REALTIME, &current_time);
	struct distance_matrix *dm = dist_matrix;
	while(dm){
		int time_elapsed = (current_time.tv_nsec - dm->last_updated.tv_nsec)/BILLION +
         (current_time.tv_sec - dm->last_updated.tv_sec);
        if(time_elapsed > MAX_UPDATE_TIMEOUT * update_interval)
			set_neighbour_cost(dm->from_node, INFINITY);
		dm = dm->next;
	}
	update_routing_distance_vector();
}

/*
 * This function resets the routing table.
 */ 
void reset_routing_distance_vector(){
	struct routing_distance_vector *rt = rt_distance_vector;
	while(rt){
		if(rt->to_node == my_id)
			rt->dist = 0;
		else
			rt->dist = INFINITY;
		rt->via_node = my_id;
		rt = rt->next;
	}
}

