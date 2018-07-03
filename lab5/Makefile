# usage:
#   on raspberry pi: make USE_RPI=yes
#   without raspberry: make

CFLAGS = -Wall -g
LDFLAGS =
LDLIBS = -lrt -lpthread

PROJECT1 = posluzitelj
OBJS1 = posluzitelj.o debug_print.o queue.o

PROJECT2 = generator
OBJS2 = generator.o

all: $(PROJECT1) $(PROJECT2)

$(PROJECT1): $(OBJS1)

$(OBJS1): debug_print.h queue.h types.h

$(PROJECT2): $(OBJS2)

$(OBJS2): types.h

clean:
	@-rm -rf $(OBJS1) $(PROJECT1) $(OBJS2) $(PROJECT2)

rmdev:
	rm /dev/shm/lab5*
	rm /dev/mqueue/lab5*
