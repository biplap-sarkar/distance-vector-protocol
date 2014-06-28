CC=gcc
CFLAGS=-c -g
AFLAG=-m32
TFLAG=-lrt

all: server
	@- echo Make successful.

server: router_simulator.o topology.o file_utils.o biplapsa_server.o
	$(CC) ${AFLAG} ${TFLAG} router_simulator.o topology.o file_utils.o biplapsa_server.o -o server

router_simulator.o: router_simulator.c
	$(CC) ${AFLAG} $(CFLAGS) router_simulator.c

topology.o: topology.c
	$(CC) ${AFLAG} $(CFLAGS) topology.c

file_utils.o: file_utils.c
	$(CC) ${AFLAG} $(CFLAGS) file_utils.c

biplapsa_server.o: biplapsa_server.c
	$(CC) ${AFLAG} $(CFLAGS) biplapsa_server.c

clean:
	rm -rf *o server
	@- echo Data Cleansing Done.Ready to Compile
