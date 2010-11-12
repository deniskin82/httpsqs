CFLAGS=-Wall -Wextra -I../ydb/root -ggdb -O3
LDFLAGS=-L../ydb/root -lydb -levent

all: httpsqs

httpsqs: httpsqs.o rbtree.o
	$(CXX) -o httpsqs $+ $(LDFLAGS) 

clean:
	rm -f httpsqs *.o

install: httpsqs
	install $(INSTALL_FLAGS) -m 4755 -o root httpsqs $(DESTDIR)/usr/bin
