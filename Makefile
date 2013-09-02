liblattice.so: liblattice.o globals.o socket.o send.o neighbors.o server_commands.o client_commands.o
	ar rvs liblattice.a liblattice.o globals.o socket.o send.o neighbors.o server_commands.o client_commands.o

liblattice.o: liblattice.c
	gcc -Wall -I./include/ -c liblattice.c

globals.o: globals.c
	gcc -Wall -I./include/ -c globals.c

socket.o: socket.c
	gcc -Wall -I./include/ -c socket.c

send.o: send.c
	gcc -Wall -I./include/ -c send.c
	
neighbors.o: neighbors.c
	gcc -Wall -I./include/ -c neighbors.c

server_commands.o: server_commands.c
	gcc -Wall -I./include/ -c server_commands.c

client_commands.o: client_commands.c
	gcc -Wall -I./include/ -c client_commands.c

clean:
	rm -rf *.o liblattice.a

