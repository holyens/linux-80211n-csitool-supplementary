CFLAGS = -O3 -Wall -static -Wno-unknown-pragmas
LDLIBS = -lorcon -lm -lrt
CC = gcc

ALL = random_packets

all: $(ALL)

clean:
	rm -f *.o $(ALL)

random_packets: random_packets.c util.o

util.c: util.h

install: random_packets setup_inject.sh setup_monitor_csi.sh
	chmod +x setup_inject.sh setup_monitor_csi.sh
	sudo ln -s `pwd`/random_packets /usr/local/bin/random_packets
	sudo ln -s `pwd`/setup_inject.sh /usr/local/bin/setup_inject.sh
	sudo ln -s `pwd`/setup_monitor_csi.sh /usr/local/bin/setup_monitor_csi.sh

uninstall:
	sudo rm -f /usr/local/bin/random_packets
	sudo rm -f /usr/local/bin/setup_inject.sh
	sudo rm -f /usr/local/bin/setup_monitor_csi.sh
