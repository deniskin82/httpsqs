# Makefile for httpsqs_client
CC=gcc

httpsqs_client: httpsqs_client.c
	$(CC) httpsqs_client.c -o httpsqs_client

clean: httpsqs_client
	rm -f httpsqs_client

install: httpsqs_client
	install -m 4755 -o root httpsqs_client $(DESTDIR)/usr/bin
