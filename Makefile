# needs Ws2_32.lib for visual studio

# source files
SOURCE = liblattice.c globals.c socket.c send.c neighbors.c server_commands.c client_commands.c sched.c forwin.c
OUTPUT = liblattice.a

CC = gcc
AR = ar rvs
CCFLAGS = -c -Wall -I./include/liblattice/
LFLAGS  = -pthread -static

CMODS = $(SOURCE)
COBJS = $(CMODS:.c=.o)

# compile each .c to .o
.c.o:
	$(CC) $(CCFLAGS) $< -o $@

all: $(COBJS)
	$(AR) $(OUTPUT) $(COBJS)

clean:
	rm -f $(COBJS) $(OUTPUT)
