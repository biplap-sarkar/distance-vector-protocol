/*
 * file_utils.h : Header file for functions to read and parse the topology file
 * 
 * Created for CSE 589 Spring 2014 Programming Assignment 3
 * 
 * @Author : Biplap Sarkar (biplapsa@buffalo.edu)
 *
 */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

/*
 * This function reads and parses the topology file
 * 
 * @args filename: topology file name
 * 
 * @returns : 0 in case of success, -1 otherwise
 * 
 */
int read_topology(char *filename);

#endif
