# usage:
#   on raspberry pi: make USE_RPI=yes
#   without raspberry: make

CFLAGS = -Wall -g
LDFLAGS =
LDLIBS = -lrt

ifeq ($(USE_RPI),yes)
CFLAGS += -D USE_RPI
LDLIBS += -lbcm2835
endif

ifeq ($(USE_RPI_SENSE),yes)
CFLAGS += -D USE_RPI_SENSE
LDLIBS += -lsense
endif

PROJECT = main
OBJS = main.o sem_io.o rpi_io.o rpi_sense.o

$(PROJECT): $(OBJS)

$(OBJS): sem_io.h rpi_io.h rpi_sense.h


debug: DEBUG = -DDEBUG
debug: main

clean:
	@-rm -rf $(OBJS) $(PROJECT) pins.txt
