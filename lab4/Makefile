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
OBJS = main.o lift.o lift_io.o lift_print.o rpi_io.o rpi_sense.o debug_print.o

$(PROJECT): $(OBJS)

$(OBJS): lift.h lift_io.h rpi_io.h rpi_sense.h debug_print.h

clean:
	@-rm -rf $(OBJS) $(PROJECT) pins.txt
