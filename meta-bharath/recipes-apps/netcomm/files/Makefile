# Makefile for netcomm (TCP/UDP client-server)
# Used by BitBake recipe via oe_runmake

CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2
LDFLAGS ?=

TARGETS = tcp-server tcp-client udp-server udp-client

all: $(TARGETS)

tcp-server: tcp-server.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

tcp-client: tcp-client.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

udp-server: udp-server.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

udp-client: udp-client.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
