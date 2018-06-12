
CC = gcc
HEADER = application_layer.h application_layer_impl.h transport_layer.h network_layer.h network_layer_impl.h randomlib.h timer.h osi.h transport_package.h transport_package_impl.h diagnostics.h
SRC = application_layer.c transport_layer.c network_layer.c randomlib.c timer.c osi.c transport_package.c program.c diagnostics.c
OPT = -Wall -Wextra -lm -g -O2 -o rtp_sim

all: $(SRC) $(HEADER)
	$(CC) $(OPT) $(SRC)
	
doc: $(SRC) $(HEADER)
	doxygen
	
clean:
	rm -rfv rtp_sim* ../doc/html/